#include "lag_monitor.h"

#include <QTimer>

LagMonitor::LagMonitor(QObject *parent) : QObject(parent)
{
    timer = new QTimer(this);
    timer->setInterval(TICK_INTERVAL_MS);
    connect(timer, &QTimer::timeout, this, &LagMonitor::checkTick);
    tickClock.start();
    timer->start();
}

QList<LagMonitor::StallRecord> LagMonitor::recentStalls() const
{
    return stalls;
}

void LagMonitor::clearStalls()
{
    stalls.clear();
}

void LagMonitor::checkTick()
{
    const qint64 gap = tickClock.restart();

    if (gap <= STALL_THRESHOLD_MS) {
        return;
    }

    StallRecord record;
    record.timestampMsSinceEpoch = QDateTime::currentMSecsSinceEpoch(); // wall time, for log correlation
    record.durationMs = gap;

    stalls.append(record);
    while (stalls.size() > MAX_RECORDED_STALLS) {
        stalls.removeFirst();
    }

    qCWarning(LagMonitorLog, "Event loop stalled for %lld ms (threshold: %d ms)", static_cast<long long>(gap),
              STALL_THRESHOLD_MS);
}
