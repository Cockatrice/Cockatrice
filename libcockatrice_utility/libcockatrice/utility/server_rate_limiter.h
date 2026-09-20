#ifndef SERVER_RATE_LIMITER_H
#define SERVER_RATE_LIMITER_H

#include <QDateTime>
#include <QMap>
#include <QString>

/**
 * @class ServerRateLimiter
 * @ingroup Utility
 * @brief Tracks per-server backoff state triggered by HTTP 429 responses.
 *
 * Keeps a monotonic backoff deadline per host and a retry-round budget that
 * escalates once per backoff window (a burst of concurrent 429s counts as a
 * single round). The budget is refreshed after a long period without 429s so a
 * server is not permanently blacklisted after a past overload.
 */
class ServerRateLimiter
{
public:
    static constexpr int MAX_RETRIES = 5;              ///< Max 429 rounds per host before the budget is exhausted
    static constexpr int MIN_429_BACKOFF_MS = 30000;   ///< Minimum wait after a 429 (scryfall documented cool-down)
    static constexpr int MAX_BACKOFF_MS = 60000;       ///< Cap for a single backoff period
    static constexpr int EXHAUSTED_BACKOFF_MS = 60000; ///< Cool-down once the retry budget is exhausted
    static constexpr qint64 RESET_GRACE_MS = 300000;   ///< Idle time after which the retry budget refreshes

    /**
     * @brief Checks whether a host is currently in backoff.
     * @param host The host to check
     * @param now The current time
     * @return True if requests to the host should be paused
     */
    [[nodiscard]] bool isRateLimited(const QString &host, const QDateTime &now) const;

    /** @return The number of consecutive 429 rounds recorded for the host. */
    [[nodiscard]] int rounds(const QString &host) const;

    /** @return The current backoff deadline for the host, or an invalid QDateTime. */
    [[nodiscard]] QDateTime deadline(const QString &host) const;

    /** @return The earliest active backoff deadline across all hosts, or an invalid QDateTime. */
    [[nodiscard]] QDateTime earliestDeadline(const QDateTime &now) const;

    /**
     * @brief Registers a 429 response for the given host.
     * @param host The host that returned 429
     * @param now The time the response was received
     * @param retryAfterMs A Retry-After hint in milliseconds, or 0 if absent
     * @return The new backoff deadline for the host
     *
     * The round counter only escalates once the previous backoff window has
     * fully passed, so concurrent 429s from a burst do not consume the whole
     * budget. The deadline is extended monotonically and never shortened.
     */
    QDateTime on429(const QString &host, const QDateTime &now, qint64 retryAfterMs = 0);

    /** @brief Clears all penalty state for a host after a successful request. */
    void onSuccess(const QString &host);

    /**
     * @brief Removes expired backoffs, refreshing the round budget of hosts
     *        that have been idle past the reset grace period.
     * @param now The current time
     */
    void clearExpired(const QDateTime &now);

private:
    QMap<QString, QDateTime> backoffUntil; ///< When each host may be contacted again
    QMap<QString, int> retryRounds;        ///< Consecutive 429 rounds per host
    QMap<QString, QDateTime> last429;      ///< When each host was last rate limited
};

#endif // SERVER_RATE_LIMITER_H
