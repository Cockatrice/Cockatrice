#ifndef RNG_SFMT_H
#define RNG_SFMT_H

#include "rng_abstract.h"
#include "sfmt/SFMT.h"

#include <QMutex>
#include <climits>

/**
 * This class encapsulates a state of the art PRNG and can be used
 * to return uniformly distributed integer random numbers from a range [min, max].
 * Though technically possible, min must be >= 0 and max should always be > 0.
 * If max < 0 and min == 0 it is assumed that rand() % -max is wanted and the result will
 * be -rand(0, -max).
 * This is the only exception to the rule that !(min > max) and is actually unused in
 * Cockatrice.
 *
 * Technical details:
 * The RNG uses the SIMD-oriented Fast Mersenne Twister code v1.4.1 from
 * http://www.math.sci.hiroshima-u.ac.jp/~%20m-mat/MT/SFMT/index.html
 * The SFMT RNG creates unsigned int 64bit pseudo random numbers.
 *
 * These are mapped to values from the interval [min, max] without bias by using Knuth's
 * "Algorithm S (Selection sampling technique)" from "The Art of Computer Programming 3rd
 * Edition Volume 2 / Seminumerical Algorithms".
 */

class RNG_SFMT : public RNG_Abstract
{
    Q_OBJECT
private:
    QMutex mutex;
    sfmt_t sfmt;
    // The discrete cumulative distribution function for the RNG
    unsigned int cdf(unsigned int min, unsigned int max);

public:
    explicit RNG_SFMT(QObject *parent = nullptr);
    unsigned int rand(int min, int max) override;
};

#endif
