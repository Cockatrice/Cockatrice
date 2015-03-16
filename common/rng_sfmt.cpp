#include "rng_sfmt.h"
#include <QDateTime>
#include <algorithm>
#include <climits>
#include <stdexcept>

// This is from gcc sources, namely from fixincludes/inclhack.def
// On C++11 systems, <cstdint> could be included instead.
#ifndef UINT64_MAX
#define UINT64_MAX (~(uint64_t)0)
#endif

RNG_SFMT::RNG_SFMT(QObject *parent)
    : RNG_Abstract(parent)
{
  // seed the random number generator using <random>, C++11
  sfmt_init_gen_rand(&sfmt, rd()); 

}

/**
 * This method is the rand() equivalent which calls the cdf with proper bounds.
 *
 * It is possible to generate random numbers from [-min, +/-max] though the RNG uses
 * unsigned numbers only, so this wrapper handles some special cases for min and max.
 *
 * It is only necessary that the upper bound is larger or equal to the lower bound - with the exception
 * that someone wants something like rand() % -foo.
 */
unsigned int RNG_SFMT::rand(int min, int max) {
  /* If min is negative, it would be possible to calculate
   * cdf(0, max - min) + min
   * There has been no use for negative random numbers with rand() though, so it's treated as error.
   */
  if(min < 0) {
    throw std::invalid_argument(
      QString("Invalid bounds for RNG: Got min " +
      QString::number(min) + " < 0!\n").toStdString());
    // at this point, the method exits. No return value is needed, because
    // basically the exception itself is returned.
  }

  // For complete fairness and equal timing, this should be a roll, but let's skip it anyway
  if(min == max)
    return max;

  // This is actually not used in Cockatrice:
  // Someone wants rand() % -foo, so we compute -rand(0, +foo)
  // This is the only time where min > max is (sort of) legal.
  // Not handling this will cause the application to crash.
  if(min == 0 && max < 0) {
    return -cdf(0, -max);
  }

  // No special cases are left, except !(min > max) which is caught in the cdf itself.
  return cdf(min, max);
}

/**
 Using SMFT random number generator on an interval from [0,1], then maping to [min,max].
 */
unsigned int RNG_SFMT::cdf(unsigned int min, unsigned int max)
{
  // This all makes no sense if min > max, which should never happen.
  if(min > max) {
    throw std::invalid_argument(
				QString("Invalid bounds for RNG: min > max! Values were: min = " +
					QString::number(min) + ", max = " +
					QString::number(max)).toStdString());
    // at this point, the method exits. No return value is needed, because
    // basically the exception itself is returned.
  }


  // generate a random number uniformly from 0 to 1
  // exclude the case when rn = 1, ( min + (max-min+1) * 1 = max + 1 )
  // numbers are generated uniformly so this should not effect the cdf
  
  double rn; 
  do {
    rn = sfmt_genrand_real1(&sfmt);
  } while ( rn == 1 ) ;

  // return the number scaled to the interval [min,max]
  return  ( min + (int) (( max -  min + 1 ) * rn));
}
