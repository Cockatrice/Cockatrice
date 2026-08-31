#include "metrics_registry.h"

#include <QList>

void MetricsRegistry::observeCommand(int typeId, qint64 elapsedMs)
{
    if (typeId < 0 || typeId >= MaxTypes) {
        typeId = MaxTypes - 1; // overflow slot keeps misrouted ids visible
    }
    if (elapsedMs < 0) {
        elapsedMs = 0;
    }

    TypeStats &stats = slotFor(typeId);
    stats.count.fetch_add(1, std::memory_order_relaxed);
    stats.totalMs.fetch_add(elapsedMs, std::memory_order_relaxed);
    totalCommandsCounter.fetch_add(1, std::memory_order_relaxed);
    totalTimeCounter.fetch_add(elapsedMs, std::memory_order_relaxed);
}

void MetricsRegistry::observeGameStartDurationMs(qint64 elapsedMs)
{
    if (elapsedMs < 0) {
        elapsedMs = 0;
    }

    gameStartStats.count.fetch_add(1, std::memory_order_relaxed);
    gameStartStats.totalMs.fetch_add(elapsedMs, std::memory_order_relaxed);
}

MetricsRegistry::TypeStats &MetricsRegistry::slotFor(int typeId)
{
    return typeSlots[static_cast<size_t>(typeId)];
}

const MetricsRegistry::TypeStats &MetricsRegistry::slotFor(int typeId) const
{
    return typeSlots[static_cast<size_t>(typeId)];
}

int MetricsRegistry::activeTypeCount() const
{
    int active = 0;
    for (int type = 0; type < MaxTypes; ++type) {
        if (slotFor(type).count.load(std::memory_order_relaxed) > 0) {
            ++active;
        }
    }
    return active;
}

QList<MetricsRegistry::ActiveTypeStats> MetricsRegistry::collectActiveStats() const
{
    QList<ActiveTypeStats> result;
    for (int type = 0; type < MaxTypes; ++type) {
        const TypeStats &stats = slotFor(type);
        const qint64 count = stats.count.load(std::memory_order_relaxed);
        if (count == 0) {
            continue;
        }
        result.append({type, count, stats.totalMs.load(std::memory_order_relaxed)});
    }
    return result;
}

MetricsRegistry::GameStartSnapshot MetricsRegistry::getGameStartSnapshot() const
{
    return {gameStartStats.count.load(std::memory_order_relaxed),
            gameStartStats.totalMs.load(std::memory_order_relaxed)};
}