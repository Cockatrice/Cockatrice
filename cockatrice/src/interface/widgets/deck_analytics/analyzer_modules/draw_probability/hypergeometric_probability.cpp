#include "hypergeometric_probability.h"

#include <QtMath>

double hypergeometricProbability(int N, int K, int n, int k)
{
    // Every rejected case here would otherwise reach qLn with a non-positive argument and
    // produce NaN. Beyond the obvious out-of-range checks, the two that matter are k > K (you
    // cannot draw more targets than exist) and n - k > N - K (drawing k targets would need more
    // non-target cards than the deck holds).
    if (k < 0 || k > n || k > K || K > N || n > N || n - k > N - K) {
        return 0.0;
    }

    double logP = 0.0;
    for (int i = 1; i <= k; ++i) {
        logP += qLn(double(K - k + i) / i);
    }
    for (int i = 1; i <= n - k; ++i) {
        logP += qLn(double(N - K - (n - k) + i) / i);
    }
    for (int i = 1; i <= n; ++i) {
        logP -= qLn(double(N - n + i) / i);
    }

    return qExp(logP);
}
