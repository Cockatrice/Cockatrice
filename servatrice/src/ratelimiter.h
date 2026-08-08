#ifndef RATELIMITER_H
#define RATELIMITER_H

#include <QList>
#include <QMap>
#include <QMutex>
#include <QString>

/** @brief Thread-safe per-key attempt counter used to throttle abusive requests. */
class RateLimiter
{
public:
    /** @brief Record an attempt for the key and report whether the key is now over the limit. */
    bool recordAttempt(const QString &key, int maxAttempts, int windowSeconds);
    /** @brief True if the key already has more than maxAttempts attempts within windowSeconds. */
    bool isBlocked(const QString &key, int maxAttempts, int windowSeconds) const;
    /** @brief Drop all recorded attempts for the key, e.g. after a successful login. */
    void clearAttempts(const QString &key);

private:
    mutable QMutex mutex;
    QMap<QString, QList<qint64>> attempts; // key -> attempt timestamps (epoch seconds)
};

#endif
