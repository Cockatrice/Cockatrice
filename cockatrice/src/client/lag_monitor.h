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

class QEvent;
class QTimer;

/**
 * @brief Detects main-thread event loop stalls ("UI freezes") from the inside.
 *
 * A timer is expected to fire every TICK_INTERVAL_MS of wall time. When the
 * observed gap greatly exceeds that interval, some other task blocked the
 * event loop for roughly the overshooting duration. This is what separates
 * "my client froze" from "the network is lagging" in user reports.
 *
 * Gaps that span an application state change (suspend, minimize, focus
 * loss) are discarded, and implausibly huge gaps are dropped, so operating
 * system power events do not fabricate stalls. This handling is load-bearing
 * on Windows, where the monotonic clock used by Qt counts sleep time.
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
        qint64 durationMs = 0; ///< approximate length of the freeze; measured tick to tick, so it can exceed the true
                               ///< stall by up to TICK_INTERVAL_MS
    };

    static constexpr int TICK_INTERVAL_MS = 500;
    static constexpr int STALL_THRESHOLD_MS = 2000;
    static constexpr int MAX_RECORDED_STALLS = 32;

    /// Gaps beyond this are treated as suspend artifacts rather than stalls.
    static constexpr qint64 MAX_PLAUSIBLE_STALL_MS = 600000;

    explicit LagMonitor(QObject *parent = nullptr);

    /**
     * @brief Stalls recorded during this session, oldest first.
     *
     * Intended consumers are log output and the diagnostics export. The list
     * holds at most MAX_RECORDED_STALLS entries.
     */
    QList<StallRecord> recentStalls() const;

    void clearStalls();

    /**
     * @brief Feeds a measured tick-to-tick gap through the detection logic.
     *
     * Split out of checkTick so threshold, plausibility, and trim behavior
     * stay unit-testable without real timing.
     */
    void recordGap(qint64 gapMs);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void checkTick();

private:
    QTimer *timer;
    QElapsedTimer tickClock; ///< monotonic clock, so wall clock steps do not fabricate stalls
    QList<StallRecord> stalls;
};

#endif
