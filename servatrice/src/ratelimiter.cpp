#include "ratelimiter.h"

#include <QDateTime>

bool RateLimiter::recordAttempt(const QString &key, int maxAttempts, int windowSeconds)
{
    return recordAttemptAt(key, maxAttempts, windowSeconds, QDateTime::currentSecsSinceEpoch());
}

bool RateLimiter::recordAttemptAt(const QString &key, int maxAttempts, int windowSeconds, qint64 now)
{
    if (maxAttempts <= 0 || windowSeconds <= 0) {
        return false; // rate limiting disabled for this endpoint
    }

    QMutexLocker locker(&mutex);
    pruneLocked(now);

    AttemptHistory &history = attempts[key];
    history.windowSeconds = windowSeconds;
    QList<qint64> &timestamps = history.timestamps;
    while (!timestamps.isEmpty() && timestamps.first() <= now - windowSeconds) {
        timestamps.removeFirst();
    }
    if (timestamps.size() >= maxAttempts) {
        // Already at the limit: reject without appending so repeated attempts
        // at the attacker's own pace cannot keep sliding the window forward
        // and hold the lockout open forever.
        return true;
    }

    timestamps.append(now);
    return false;
}

bool RateLimiter::isBlocked(const QString &key, int maxAttempts, int windowSeconds) const
{
    return isBlockedAt(key, maxAttempts, windowSeconds, QDateTime::currentSecsSinceEpoch());
}

bool RateLimiter::isBlockedAt(const QString &key, int maxAttempts, int windowSeconds, qint64 now) const
{
    if (maxAttempts <= 0 || windowSeconds <= 0) {
        return false;
    }

    QMutexLocker locker(&mutex);

    const auto it = attempts.constFind(key);
    if (it == attempts.constEnd()) {
        return false;
    }

    int count = 0;
    for (const qint64 &timestamp : it.value().timestamps) {
        if (timestamp > now - windowSeconds) {
            ++count;
        }
    }
    return count >= maxAttempts;
}

void RateLimiter::clearAttempts(const QString &key)
{
    QMutexLocker locker(&mutex);
    attempts.remove(key);
}

void RateLimiter::pruneLocked(qint64 now)
{
    if (lastPruneSecs + PruneIntervalSeconds > now) {
        return;
    }
    lastPruneSecs = now;

    auto it = attempts.begin();
    while (it != attempts.end()) {
        QList<qint64> &timestamps = it.value().timestamps;
        while (!timestamps.isEmpty() && timestamps.first() <= now - it.value().windowSeconds) {
            timestamps.removeFirst();
        }
        if (timestamps.isEmpty()) {
            it = attempts.erase(it);
        } else {
            ++it;
        }
    }
}