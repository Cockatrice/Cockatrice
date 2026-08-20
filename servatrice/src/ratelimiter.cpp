#include "ratelimiter.h"

#include <QDateTime>

bool RateLimiter::recordAttempt(const QString &key, int maxAttempts, int windowSeconds)
{
    QMutexLocker locker(&mutex);
    const qint64 now = QDateTime::currentSecsSinceEpoch();

    QList<qint64> &timestamps = attempts[key];
    timestamps.append(now);
    while (!timestamps.isEmpty() && timestamps.first() <= now - windowSeconds) {
        timestamps.removeFirst();
    }

    return timestamps.size() > maxAttempts;
}

bool RateLimiter::isBlocked(const QString &key, int maxAttempts, int windowSeconds) const
{
    QMutexLocker locker(&mutex);
    const qint64 now = QDateTime::currentSecsSinceEpoch();

    const auto it = attempts.constFind(key);
    if (it == attempts.constEnd()) {
        return false;
    }

    int count = 0;
    for (const qint64 &timestamp : it.value()) {
        if (timestamp > now - windowSeconds) {
            ++count;
        }
    }
    return count > maxAttempts;
}

void RateLimiter::clearAttempts(const QString &key)
{
    QMutexLocker locker(&mutex);
    attempts.remove(key);
}
