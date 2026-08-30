/**
 * @file metrics_registry.h
 * @ingroup Servatrice
 */

#ifndef METRICS_REGISTRY_H
#define METRICS_REGISTRY_H

#include <QList>
#include <QString>
#include <array>
#include <atomic>

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
 *
 * Only counts and totals are retained. An earlier Prometheus-style cumulative
 * histogram (per-type, time-bucketed) was cut because nothing in the server
 * ever wrote it out; it belongs to the future /metrics exporter that needs it.
 */
class MetricsRegistry
{
public:
    /**
     * Extension numbers are only unique per command kind, so recorded ids
     * combine the kind index with the protobuf extension number.
     *
     * The stride is only as wide as it needs to be: 1280 is the first round
     * number above the largest extension actually in use (ModeratorCommand =
     * 1206) and keeps the preallocated TypeStats array small. Bump it if a new
     * command exceeds it.
     */
    static constexpr int KindStride = 1280;

    static constexpr int NumKinds = 6;

    static constexpr const char *KindNames[NumKinds] = {"session", "room", "game", "moderator", "admin", "developer"};

    /// Upper bound on distinct command type ids (see typeIdFor).
    static constexpr int MaxTypes = NumKinds * KindStride;

    /// Guard against typeIdFor() overflowing into the neighbouring kind's slots.
    static_assert(KindStride > 1206, "KindStride must exceed the highest command extension number in use");

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
     * Callers resolve the numeric type id to a human-readable label via
     * typeIdFor()/KindNames as needed.
     */
    QList<ActiveTypeStats> collectActiveStats() const;

    struct GameStartSnapshot
    {
        qint64 count;
        qint64 totalMs;
    };

    GameStartSnapshot getGameStartSnapshot() const;

private:
    struct TypeStats
    {
        std::atomic<qint64> count{0};
        std::atomic<qint64> totalMs{0};
    };

    TypeStats &slotFor(int typeId);
    const TypeStats &slotFor(int typeId) const;

    std::array<TypeStats, MaxTypes> typeSlots{};
    TypeStats gameStartStats{};
    std::atomic<qint64> totalCommandsCounter{0};
    std::atomic<qint64> totalTimeCounter{0};
};

#endif