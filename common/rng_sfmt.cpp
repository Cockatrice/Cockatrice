#include "rng_sfmt.h"
#include <QDateTime>
#include <algorithm>

// This is from gcc sources, namely from fixincludes/inclhack.def
// On C++11 systems, <cstdint> could be included instead.
#define UINT64_MAX (~(uint64_t)0)

RNG_SFMT::RNG_SFMT(QObject *parent)
	: RNG_Abstract(parent)
{
	// initialize the random number generator with a 32bit integer seed (timestamp)
	sfmt_init_gen_rand(&sfmt, QDateTime::currentDateTime().toTime_t());
}

/**
 * Much thought went into this, please read this comment before you modify the code.
 * Let SFMT() be an alias for sfmt_genrand_uint64() aka SFMT's rand() function.
 *
 * SMFT() returns a uniformly distributed pseudorandom number from 0 to UINT64_MAX.
 * As SFMT() operates on a limited integer range, it is a _discrete_ function.
 *
 * We want a random number from a given interval [min, max] though, so we need to
 * implement the (discrete) cumulative distribution function SFMT(min, max), which
 * returns a random number X from [min, max].
 *
 * This CDF is by formal definition:
 * SFMT(X; min, max) = (floor(X) - min + 1) / (max - min + 1)
 *
 * To get out the random variable, solve for X:
 * floor(X) = SFMT(X; min, max) * (max - min + 1) + min - 1
 * So this is, what getNumber(min, max) should look like.
 * Problem: SFMT(X; min, max) * (max - min + 1) could produce an integer overflow,
 * so it is not safe.
 *
 * One solution is to divide the universe into buckets of equal size depending on the
 * range [min, max] and assign X to the bucket that contains the number generated
 * by SFMT(). This equals to modulo computation and is not satisfying:
 * If the buckets don't divide the universe equally, because the bucket size is not
 * a divisor of 2, there will be a range in the universe that is biased because one
 * bucket is too small thus will be chosen less equally!
 *
 * This is solved by rejection sampling:
 * As SFMT() is assumed to be unbiased, we are allowed to ignore those random numbers
 * from SFMT() that would force us to have an unequal bucket and generate new random
 * numbers until one number fits into one of the other buckets.
 * This can be compared to an ideal six sided die that is rolled until only sides
 * 1-5 show up, while 6 represents something that you don't want. So you basically roll
 * a five sided die.
 * 
 * Note: If you replace the SFMT RNG with some other rand() function in the future,
 * then you _need_ to change the UINT64_MAX constant to the largest possible random 
 * number which can be created by the new rand() function. This value is often defined
 * in a RAND_MAX constant.
 * Otherwise you will probably skew the outcome of the getNumber() method or worsen the
 * performance of the application.
 */
unsigned int RNG_SFMT::getNumber(unsigned int min, unsigned int max)
{
	// This all makes no sense if min > max.
	// So in debug mode Q_ASSERT will print a warning...
	Q_ASSERT(min <= max);
	// ... and at runtime min and max will be swapped; this should never happen.
	if(min > max)
	  std::swap(min, max);

	// First compute the diameter (aka size, length) of the [min, max] interval
	const unsigned int diameter = max - min + 1;

	// Compute how many buckets (each in size of the diameter) will fit into the
	// universe.
	// If the division has a remainder, the result is floored automatically.
	const uint64_t buckets = UINT64_MAX / diameter;

	// Compute the last valid random number. All numbers beyond have to be ignored.
	// If there was no remainder in the previous step, limit is equal to UINT64_MAX.
	const uint64_t limit = diameter * buckets;

	uint64_t rand;
	// To make the random number generation thread-safe, a mutex is created around
	// the generation. Outside of the loop of course, to avoid lock/unlock overhead.
	mutex.lock();
	do {
		rand = sfmt_genrand_uint64(&sfmt);
	} while (rand >= limit);
	mutex.unlock();

	// Now determine the bucket containing the SFMT() random number and after adding
	// the lower bound, a random number from [min, max] can be returned.
	return (unsigned int) (rand / buckets + min);
}
