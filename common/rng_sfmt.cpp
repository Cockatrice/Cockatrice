#include "rng_sfmt.h"
#include <QDateTime>
#include <stdlib.h>
#include <iostream>

RNG_SFMT::RNG_SFMT(QObject *parent)
	: RNG_Abstract(parent)
{
	// initialize the random number generator with a 32bit integer seed (timestamp)
	sfmt_init_gen_rand(&sfmt, QDateTime::currentDateTime().toTime_t());
}

unsigned int RNG_SFMT::getNumber(unsigned int min, unsigned int max)
{
	// To make the random number generation thread safe, a mutex is created around the generation.
	mutex.lock();
	uint64_t r = sfmt_genrand_uint64(&sfmt);
	mutex.unlock();
	
	// return a random number from the interval [min, max]
	return (unsigned int) (r % (max - min + 1) + min);
}
