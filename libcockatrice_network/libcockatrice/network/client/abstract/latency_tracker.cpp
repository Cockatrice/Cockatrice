#include "latency_tracker.h"

#include <QtMath>
#include <algorithm>

void LatencyTracker::addSample(qint64 ms)
{
    samples[static_cast<size_t>(head)] = ms;
    head = (head + 1) % WindowSize;
    if (count < WindowSize) {
        ++count;
    }
}

QList<qint64> LatencyTracker::recentSamples() const
{
    QList<qint64> result;
    result.reserve(count);
    for (int i = count; i > 0; --i) {
        const int index = (head + WindowSize - i) % WindowSize;
        result.append(samples[static_cast<size_t>(index)]);
    }
    return result;
}

LatencyTracker::Stats LatencyTracker::stats() const
{
    if (count == 0) {
        return {};
    }

    QList<qint64> sorted(samples.cbegin(), samples.cbegin() + count);
    std::sort(sorted.begin(), sorted.end());

    Stats s;
    s.sampleCount = count;
    s.lastMs = samples[static_cast<size_t>((head + WindowSize - 1) % WindowSize)];
    s.maxMs = sorted.last();

    const int n = count;
    if (n % 2 == 1) {
        s.medianMs = sorted[n / 2];
    } else {
        s.medianMs = (sorted[n / 2 - 1] + sorted[n / 2]) / 2;
    }

    // Nearest-rank percentile: smallest value in the list such that at least
    // 95% of the samples are <= it.
    const int p95Index = qMax(0, qCeil(0.95 * static_cast<double>(n)) - 1);
    s.p95Ms = sorted[p95Index];

    return s;
}

void LatencyTracker::clear()
{
    samples.fill(0);
    head = 0;
    count = 0;
}
