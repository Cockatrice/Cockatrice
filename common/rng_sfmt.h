#ifndef RNG_SFMT_H
#define RNG_SFMT_H

#include "sfmt/SFMT.h"
#include "rng_abstract.h"
#include <QMutex>

/**
 * This class represents the random number generator.
 * Usage instructions:
 * Create an instance of RNG_SFMT, then call getNumber(min, max).
 * You should never call this function with min > max, this throws a
 * std::invalid_argument exception. It is best practice to use getNumber() in a try block
 * and catch the exception if min, max are entered by the user or computed somehow.
 *
 * Technical details:
 * The RNG uses the SIMD-oriented Fast Mersenne Twister code v1.4.1 from
 * http://www.math.sci.hiroshima-u.ac.jp/~%20m-mat/MT/SFMT/index.html
 * To use this RNG, the class needs a sfmt_t structure for the RNG's internal state.
 * It has to be initialized by sfmt_init_gen_rand() which is done in the constructor.
 * The function sfmt_genrand_uint64() can then be used to create a 64 bit unsigned int 
 * pseudorandom number. This is done in getNumber().
 * For more information see the author's website and look at the documentation and 
 * examples that are part of the official downloads.
 */

class RNG_SFMT : public RNG_Abstract {
	Q_OBJECT
private:
	QMutex mutex;
	sfmt_t sfmt;
public:
	RNG_SFMT(QObject *parent = 0);
	unsigned int getNumber(unsigned int min, unsigned int max);
};

#endif
 
