#ifndef COCKATRICE_HYPERGEOMETRIC_PROBABILITY_H
#define COCKATRICE_HYPERGEOMETRIC_PROBABILITY_H

/** @brief Probability of drawing exactly k of K target cards in a hand of n from a population of N.
 *
 *  Returns 0.0 for arguments that cannot describe a draw, which includes k > K: you cannot draw
 *  more target cards than the deck holds, and asking for it yields log of a negative term rather
 *  than a probability.
 */
double hypergeometricProbability(int N, int K, int n, int k);

#endif // COCKATRICE_HYPERGEOMETRIC_PROBABILITY_H
