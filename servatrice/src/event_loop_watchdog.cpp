/**
 * @file event_loop_watchdog.cpp
 * @ingroup Servatrice
 */

#include "event_loop_watchdog.h"

#include "servatrice.h"

#include <QTimer>

EventLoopWatchdog::EventLoopWatchdog(Servatrice *_servatrice, QString _threadName)
    : QObject(nullptr), servatrice(_servatrice), threadName(std::move(_threadName))
{
}

void EventLoopWatchdog::start()
{
    heartbeatTimer = new QTimer(this);
    sinceLastTick.start();
    connect(heartbeatTimer, &QTimer::timeout, this, &EventLoopWatchdog::checkHeartbeat);
    heartbeatTimer->start(HeartbeatIntervalMs);
}

void EventLoopWatchdog::checkHeartbeat()
{
    const qint64 elapsedMs = sinceLastTick.restart();
    const qint64 overshootMs = qMax<qint64>(0, elapsedMs - HeartbeatIntervalMs);
    if (overshootMs < servatrice->getMetricsStallWarnMs()) {
        return;
    }

    servatrice->observeEventLoopStall(threadName, overshootMs);
}
