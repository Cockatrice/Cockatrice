/**
 * @file lag_monitor.h
 * @ingroup Client
 */

#ifndef LAG_MONITOR_H
#define LAG_MONITOR_H

#include <QDateTime>
#include <QElapsedTimer>
#include <QList>
#include <QLoggingCategory>
#include <QObject>

inline Q_LOGGING_CATEGORY(LagMonitorLog, "lag_monitor");

class QTimer;

/**
 * @brief Detects main-thread event loop stalls ("UI freezes") from the inside.
 *
 * A timer is expected to fire every TickIntervalMs of wall time. When the
 * observed gap greatly exceeds that interval, some other task blocked the
 * event loop for roughly the overshooting duration. This is what separates
 * "my client froze" from "the network is lagging" in user reports.
 *
 * Healthy operation costs one timer wakeup per tick and two integer
 * comparisons. Allocations happen only when a stall is actually recorded.
 */
class LagMonitor : public QObject
{
    Q_OBJECT

public:
    struct StallRecord
    {
        qint64 timestampMsSinceEpoch = 0; ///< when the stalled period ended
        qint64 durationMs = 0;            ///< approximate length of the freeze
    };

    static constexpr int TickIntervalMs = 500;
    static constexpr int StallThresholdMs = 2000;
    static constexpr int MaxRecordedStalls = 32;

    explicit LagMonitor(QObject *parent = nullptr);

    /**
     * @brief Stalls recorded during this session, oldest first.
     *
     * Intended consumers are log output and the diagnostics export. The list
     * holds at most MaxRecordedStalls entries.
     */
    QList<StallRecord> recentStalls() const;

    void clearStalls();

private slots:
    void checkTick();

private:
    QTimer *timer;
    QElapsedTimer tickClock; ///< monotonic clock, so wall clock steps and suspend do not fabricate stalls
    QList<StallRecord> stalls;
};

#endif
