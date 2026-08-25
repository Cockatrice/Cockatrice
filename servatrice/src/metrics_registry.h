/**
 * @file metrics_registry.h
 * @ingroup Servatrice
 */

#ifndef METRICS_REGISTRY_H
#define METRICS_REGISTRY_H

#include <QHash>
#include <QString>
#include <array>
#include <atomic>
#include <functional>

/**
 * @brief Lock-free accumulation of command processing statistics.
 *
 * observeCommand() is called once per processed command from whichever socket
 * thread handled it. It uses relaxed atomic adds on preallocated storage only,
 * so it introduces no locks, allocations, or shared cache-line ping-pong
 * beyond the unavoidable counter updates.
 *
 * Reading happens rarely (metrics scraping), accepts momentary tears between
 * related counters, and therefore also needs no synchronization.
 */
class MetricsRegistry
{
public:
    /**
     * Extension numbers are only unique per command kind, so recorded ids
     * combine the kind index with the protobuf extension number.
     */
    static constexpr int KindStride = 2048;

    static constexpr int NumKinds = 5;

    static constexpr const char *KindNames[NumKinds] = {"session", "room", "game", "moderator", "admin"};

    /// Upper bound on distinct command type ids (see typeIdFor).
    static constexpr int MaxTypes = NumKinds * KindStride;

    /// Histogram bucket upper bounds in milliseconds. Anything above the last
    /// bound lands in the trailing +Inf bucket. Constexpr so the recording
    /// hot path never allocates.
    static constexpr std::array<qint64, 11> BucketBounds{1, 5, 10, 25, 50, 100, 250, 500, 1000, 2500, 5000};

    static int typeIdFor(int kindIndex, int extensionNumber)
    {
        return kindIndex * KindStride + extensionNumber;
    }

    void observeCommand(int typeId, qint64 elapsedMs);

    /**
     * Records how long one game start took to bring every player's zones
     * online. Kept separate from command timings because it is triggered by
     * the server itself and can dwarf any single command when decks are huge.
     */
    void observeGameStartDurationMs(qint64 elapsedMs);

    /// Total number of observed commands across all types.
    qint64 totalCommands() const
    {
        return totalCommandsCounter.load(std::memory_order_relaxed);
    }

    /// Cumulative processing milliseconds across all types.
    qint64 totalTimeMs() const
    {
        return totalTimeCounter.load(std::memory_order_relaxed);
    }

    /// Number of distinct type slots that have seen at least one sample.
    int activeTypeCount() const;

    struct ActiveTypeStats
    {
        int typeId;
        qint64 count;
        qint64 totalMs;
    };

    /**
     * Returns stats for every type slot that has seen at least one sample.
     * The caller-provided @p labelForType maps a numeric type id to a stable
     * human-readable label. Pass nullptr to skip label resolution.
     */
    QList<ActiveTypeStats> collectActiveStats() const;

    struct GameStartSnapshot
    {
        qint64 count;
        qint64 totalMs;
    };

    GameStartSnapshot getGameStartSnapshot() const;

    /**
     * @brief Renders all recorded data in Prometheus text exposition format.
     *
     * @param nameForType maps a numeric command type id to a stable label
     * value. Ids without a mapping are rendered as their number.
     * @param gauges simple name/value pairs emitted as gauge samples.
     */
    QString toPrometheusText(const std::function<QString(int)> &nameForType,
                             const QHash<QString, qint64> &gauges) const;

private:
    static constexpr int BucketCount = static_cast<int>(BucketBounds.size()) + 1; ///< bounds + the +Inf bucket

    struct TypeStats
    {
        std::atomic<qint64> count{0};
        std::atomic<qint64> totalMs{0};
        std::array<std::atomic<qint64>, BucketCount> buckets{};
    };

    TypeStats &slotFor(int typeId);
    const TypeStats &slotFor(int typeId) const;

    /// Index of the histogram bucket the sample falls into. The last index is +Inf.
    static int bucketIndexFor(qint64 elapsedMs);

    /// Appends one series of cumulative +Inf-terminated buckets to @p out.
    static void appendCumulativeBuckets(QString &out,
                                        const QString &bucketLine,
                                        const std::array<std::atomic<qint64>, BucketCount> &buckets);

    std::array<TypeStats, MaxTypes> typeSlots{};
    TypeStats gameStartStats{};
    std::atomic<qint64> totalCommandsCounter{0};
    std::atomic<qint64> totalTimeCounter{0};
};

#endif
