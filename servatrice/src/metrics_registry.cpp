#include "metrics_registry.h"

#include <QList>

int MetricsRegistry::bucketIndexFor(qint64 elapsedMs)
{
    const int lastFiniteBucket = static_cast<int>(BucketBounds.size()) - 1;
    int bucket = 0;
    while (bucket < lastFiniteBucket && elapsedMs > BucketBounds[static_cast<size_t>(bucket)]) {
        ++bucket;
    }
    return bucket;
}

void MetricsRegistry::appendCumulativeBuckets(QString &out,
                                              const QString &bucketLine,
                                              const std::array<std::atomic<qint64>, BucketCount> &buckets)
{
    // Cumulative buckets are required by the Prometheus histogram convention.
    qint64 cumulative = 0;
    for (int bucket = 0; bucket < static_cast<int>(BucketBounds.size()); ++bucket) {
        cumulative += buckets[static_cast<size_t>(bucket)].load(std::memory_order_relaxed);
        out += QStringLiteral("%1,le=\"%2\"} %3\n")
                   .arg(bucketLine)
                   .arg(BucketBounds[static_cast<size_t>(bucket)])
                   .arg(cumulative);
    }
    cumulative += buckets[BucketCount - 1].load(std::memory_order_relaxed);
    out += QStringLiteral("%1,le=\"+Inf\"} %2\n").arg(bucketLine).arg(cumulative);
}

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
    stats.buckets[static_cast<size_t>(bucketIndexFor(elapsedMs))].fetch_add(1, std::memory_order_relaxed);
}

void MetricsRegistry::observeGameStartDurationMs(qint64 elapsedMs)
{
    if (elapsedMs < 0) {
        elapsedMs = 0;
    }

    gameStartStats.count.fetch_add(1, std::memory_order_relaxed);
    gameStartStats.totalMs.fetch_add(elapsedMs, std::memory_order_relaxed);
    gameStartStats.buckets[static_cast<size_t>(bucketIndexFor(elapsedMs))].fetch_add(1, std::memory_order_relaxed);
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

QString MetricsRegistry::toPrometheusText(const std::function<QString(int)> &nameForType,
                                          const QHash<QString, qint64> &gauges) const
{
    QString out;
    out.reserve(4096);

    for (auto it = gauges.constBegin(); it != gauges.constEnd(); ++it) {
        out += QStringLiteral("# TYPE %1 gauge\n").arg(it.key());
        out += QStringLiteral("%1 %2\n").arg(it.key()).arg(it.value());
    }

    // Cumulative buckets are required by the Prometheus histogram convention.
    out += QLatin1String("# TYPE servatrice_commands_duration_ms histogram\n");

    for (int type = 0; type < MaxTypes; ++type) {
        const TypeStats &stats = slotFor(type);
        const qint64 count = stats.count.load(std::memory_order_relaxed);
        if (count == 0) {
            continue;
        }

        const QString label = nameForType ? nameForType(type) : QString::number(type);
        appendCumulativeBuckets(out, QStringLiteral("servatrice_commands_duration_ms_bucket{command=\"%1\"").arg(label),
                                stats.buckets);

        out += QStringLiteral("servatrice_commands_duration_ms_sum{command=\"%1\"} %2\n")
                   .arg(label)
                   .arg(stats.totalMs.load(std::memory_order_relaxed));
        out += QStringLiteral("servatrice_commands_duration_ms_count{command=\"%1\"} %2\n").arg(label).arg(count);
    }

    const qint64 startCount = gameStartStats.count.load(std::memory_order_relaxed);
    if (startCount > 0) {
        out += QLatin1String("# TYPE servatrice_game_start_duration_ms histogram\n");
        appendCumulativeBuckets(out, QLatin1String("servatrice_game_start_duration_ms_bucket"), gameStartStats.buckets);
        out += QStringLiteral("servatrice_game_start_duration_ms_sum %1\n")
                   .arg(gameStartStats.totalMs.load(std::memory_order_relaxed));
        out += QStringLiteral("servatrice_game_start_duration_ms_count %1\n").arg(startCount);
    }

    return out;
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
