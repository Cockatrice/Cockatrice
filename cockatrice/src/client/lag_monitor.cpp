#include "lag_monitor.h"

#include <QCoreApplication>
#include <QEvent>
#include <QTimer>

LagMonitor::LagMonitor(QObject *parent) : QObject(parent)
{
    qApp->installEventFilter(this);

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

bool LagMonitor::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::ApplicationStateChange) {
        // The transition may span a suspend or an arbitrary unfocused period;
        // discard the gap so it cannot be mistaken for a stall.
        tickClock.restart();
    }
    return QObject::eventFilter(obj, event);
}

void LagMonitor::checkTick()
{
    recordGap(tickClock.restart());
}

void LagMonitor::recordGap(qint64 gapMs)
{
    if (gapMs <= STALL_THRESHOLD_MS) {
        return;
    }

    if (gapMs > MAX_PLAUSIBLE_STALL_MS) {
        qCDebug(LagMonitorLog, "Ignoring implausible %lld ms gap (likely suspend)", static_cast<long long>(gapMs));
        return;
    }

    const StallRecord record{.timestampMsSinceEpoch = QDateTime::currentMSecsSinceEpoch(), .durationMs = gapMs};

    stalls.append(record);
    while (stalls.size() > MAX_RECORDED_STALLS) {
        stalls.removeFirst();
    }

    qCWarning(LagMonitorLog, "Event loop stalled for %lld ms (threshold: %d ms)", static_cast<long long>(gapMs),
              STALL_THRESHOLD_MS);
}
