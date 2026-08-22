/**
 * @file latency_tracker.h
 * @ingroup Client
 */

#ifndef LATENCY_TRACKER_H
#define LATENCY_TRACKER_H

#include <QList>
#include <QMetaType>
#include <array>

/**
 * @brief Fixed-capacity rolling window of network round-trip time samples.
 *
 * The hot path (addSample) is a single array store and is intentionally free of
 * allocations, locks, or signal emissions so that recording one sample per
 * completed command cannot affect gameplay performance. Aggregate statistics
 * are only computed on demand in stats(), which callers should throttle.
 */
class LatencyTracker
{
public:
    static constexpr int WindowSize = 64;

    struct Stats
    {
        qint64 lastMs = 0;   ///< most recently added sample
        qint64 medianMs = 0; ///< median over the current window
        qint64 p95Ms = 0;    ///< 95th percentile over the current window
        qint64 maxMs = 0;    ///< maximum over the current window
        int sampleCount = 0; ///< number of samples currently in the window
    };

    void addSample(qint64 ms);
    Stats stats() const;

    /// Snapshot of the current window in chronological order (oldest first).
    QList<qint64> recentSamples() const;

    void clear();

private:
    std::array<qint64, WindowSize> samples{};
    int head = 0;  ///< index where the next sample will be written
    int count = 0; ///< number of valid samples, capped at WindowSize
};

Q_DECLARE_METATYPE(LatencyTracker::Stats)

#endif
