#ifndef RATELIMITER_H
#define RATELIMITER_H

#include <QList>
#include <QMap>
#include <QMutex>
#include <QString>

/** @brief Thread-safe per-key attempt limiter used to throttle abusive requests. */
class RateLimiter
{
public:
    /**
     * @brief Record an attempt for @p key and report whether the key is now blocked.
     *
     * A key that is already at its limit is not extended: the attempt is
     * rejected without appending, so the lockout lifts once the recorded
     * attempts age out of the window. Returns false immediately when
     * @p maxAttempts <= 0 or @p windowSeconds <= 0 (rate limiting disabled),
     * recording nothing.
     *
     * Delegates to recordAttemptAt() with the current wall-clock time.
     */
    bool recordAttempt(const QString &key, int maxAttempts, int windowSeconds);

    /**
     * @brief Record an attempt as seen at epoch second @p now.
     *
     * Exposed for deterministic tests; production callers should use
     * recordAttempt().
     */
    bool recordAttemptAt(const QString &key, int maxAttempts, int windowSeconds, qint64 now);

    /**
     * @brief True if @p key already has at least @p maxAttempts attempts within @p windowSeconds.
     *
     * Reads only; records nothing. Returns false when @p maxAttempts <= 0 or
     * @p windowSeconds <= 0. Delegates to isBlockedAt().
     */
    bool isBlocked(const QString &key, int maxAttempts, int windowSeconds) const;

    /**
     * @brief Whether @p key is blocked as measured at epoch second @p now.
     *
     * Exposed for deterministic tests; production callers should use
     * isBlocked().
     */
    bool isBlockedAt(const QString &key, int maxAttempts, int windowSeconds, qint64 now) const;

    /** @brief Drop all recorded attempts for the key, e.g. after a successful login. */
    void clearAttempts(const QString &key);

private:
    struct AttemptHistory
    {
        QList<qint64> timestamps; // epoch seconds, oldest first
        int windowSeconds;
    };

    /**
     * @brief Drops keys whose last attempts have fully aged out of their window.
     *
     * Runs at most every PruneIntervalSeconds to stop the map from growing
     * without bound when attackers rotate source addresses.
     */
    void pruneLocked(qint64 now);

    mutable QMutex mutex;
    QMap<QString, AttemptHistory> attempts; // key -> attempt history
    qint64 lastPruneSecs = 0;
    static constexpr int PruneIntervalSeconds = 60;
};

#endif