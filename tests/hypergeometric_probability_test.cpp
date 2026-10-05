/** @file hypergeometric_probability_test.cpp
 *  @brief Tests for the draw-probability hypergeometric calculation.
 *  @ingroup Tests
 */

#include <cmath>
#include <gtest/gtest.h>
#include <hypergeometric_probability.h>
#include <string>

// Drawing more target cards than the deck holds has no valid outcome. Without a k > K guard the
// running product reaches qLn of a negative term, so the result is NaN and the widget prints "nan".
TEST(HypergeometricProbability, AskingForMoreCopiesThanTheDeckHoldsIsZero)
{
    EXPECT_DOUBLE_EQ(hypergeometricProbability(60, 1, 7, 3), 0.0);
    EXPECT_DOUBLE_EQ(hypergeometricProbability(60, 2, 7, 4), 0.0);
    EXPECT_DOUBLE_EQ(hypergeometricProbability(4, 1, 1, 2), 0.0);
}

// Drawing k targets from n cards needs n - k non-targets, and the deck may not hold that many.
// An 8-card deck with 6 targets leaves 2 non-targets, so drawing 5 cards with 0 targets would
// need 5 of them. The n - k term goes negative, which is the same NaN through a different factor.
TEST(HypergeometricProbability, AskingForMoreNonTargetsThanTheDeckHoldsIsZero)
{
    EXPECT_DOUBLE_EQ(hypergeometricProbability(8, 6, 5, 0), 0.0);
    EXPECT_DOUBLE_EQ(hypergeometricProbability(8, 6, 5, 1), 0.0);
    EXPECT_DOUBLE_EQ(hypergeometricProbability(10, 9, 8, 0), 0.0);
}

// k == K is the reachable edge of the previous case: drawing every target card exactly. It must
// stay valid, so the guard cannot be written as k >= K.
TEST(HypergeometricProbability, DrawingEveryCopyIsNotRejected)
{
    // One Forest in a 60-card deck, drawing 7: P(the Forest) = 7/60.
    EXPECT_NEAR(hypergeometricProbability(60, 1, 7, 1), 7.0 / 60.0, 1e-12);
    EXPECT_GT(hypergeometricProbability(60, 4, 7, 4), 0.0);
    // The only way to fill a hand from a deck the size of the hand is to take everything.
    EXPECT_NEAR(hypergeometricProbability(4, 2, 4, 2), 1.0, 1e-12);
}

// k == 0 with n == 0 draws nothing at all, which is certain.
TEST(HypergeometricProbability, DrawingAnEmptyHandIsCertain)
{
    EXPECT_NEAR(hypergeometricProbability(60, 4, 0, 0), 1.0, 1e-12);
    EXPECT_NEAR(hypergeometricProbability(4, 2, 0, 0), 1.0, 1e-12);
}

TEST(HypergeometricProbability, ImpossibleDrawsAreZero)
{
    EXPECT_DOUBLE_EQ(hypergeometricProbability(60, 4, 7, 9), 0.0);  // k > n
    EXPECT_DOUBLE_EQ(hypergeometricProbability(10, 12, 5, 3), 0.0); // K > N
    EXPECT_DOUBLE_EQ(hypergeometricProbability(10, 4, 20, 5), 0.0); // n > N
    EXPECT_DOUBLE_EQ(hypergeometricProbability(10, 4, 5, -1), 0.0); // k < 0
}

// The sweep that motivated both guards: no argument combination may produce NaN. The upper bound
// is 1.0 plus a few ulps, since the sum of logarithms accumulates rounding error on a certain
// outcome rather than landing on exactly 1.
TEST(HypergeometricProbability, ProbabilitiesAreFiniteAndInRange)
{
    for (int N = 1; N <= 70; N += 7) {
        for (int K = 0; K <= N; K += 3) {
            for (int n = 0; n <= N; n += 5) {
                for (int k = 0; k <= n; ++k) {
                    const double p = hypergeometricProbability(N, K, n, k);
                    const std::string at = "N=" + std::to_string(N) + " K=" + std::to_string(K) +
                                           " n=" + std::to_string(n) + " k=" + std::to_string(k);
                    ASSERT_FALSE(std::isnan(p)) << at;
                    ASSERT_GE(p, 0.0) << at;
                    ASSERT_LE(p, 1.0 + 1e-12) << at;
                }
            }
        }
    }
}

// A known value: the chance of drawing both copies of a 2-of-4 card in a 7-card hand from a
// 60-card deck. C(4,2)*C(56,5)/C(60,7).
TEST(HypergeometricProbability, KnownValueMatchesTheClosedForm)
{
    const double expected = (6.0 * 3819816.0) / 386206920.0;
    EXPECT_NEAR(hypergeometricProbability(60, 4, 7, 2), expected, 1e-9);
}
