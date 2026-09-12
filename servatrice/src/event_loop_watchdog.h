/**
 * @file event_loop_watchdog.h
 * @ingroup Servatrice
 */

#ifndef EVENT_LOOP_WATCHDOG_H
#define EVENT_LOOP_WATCHDOG_H

#include <QElapsedTimer>
#include <QObject>
#include <QString>

class Servatrice;
class QTimer;

/**
 * @brief Detects blocked or overloaded worker event loops.
 *
 * One instance lives in each socket pool thread. A heartbeat timer tick that
 * arrives late means the loop spent that time elsewhere: busy work, a queued
 * slot, or a hard wedge. Overshoots past the configured threshold bump
 * lock-free counters on the metrics registry and log one warning per stall,
 * so a stuck pool thread becomes visible instead of silent lag.
 */
class EventLoopWatchdog : public QObject
{
    Q_OBJECT
public:
    /// How often the heartbeat expects to fire. Small enough to catch short stalls.
    static constexpr int HeartbeatIntervalMs = 500;

    EventLoopWatchdog(Servatrice *_servatrice, QString _threadName);

    /**
     * Starts the heartbeat timer. Must be invoked queued after the instance
     * was moved to its target thread so the timer lives there too.
     */
    void start();

private slots:
    void checkHeartbeat();

private:
    Servatrice *servatrice;
    QString threadName;
    QElapsedTimer sinceLastTick;
    QTimer *heartbeatTimer = nullptr;
};

#endif
