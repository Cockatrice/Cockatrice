#include "server_rate_limiter.h"

bool ServerRateLimiter::isRateLimited(const QString &host, const QDateTime &now) const
{
    auto it = backoffUntil.constFind(host);
    return it != backoffUntil.constEnd() && now < it.value();
}

int ServerRateLimiter::rounds(const QString &host) const
{
    return retryRounds.value(host, 0);
}

QDateTime ServerRateLimiter::deadline(const QString &host) const
{
    return backoffUntil.value(host);
}

QDateTime ServerRateLimiter::earliestDeadline(const QDateTime &now) const
{
    QDateTime earliest;
    for (auto it = backoffUntil.cbegin(); it != backoffUntil.cend(); ++it) {
        if (now < it.value() && (!earliest.isValid() || it.value() < earliest)) {
            earliest = it.value();
        }
    }
    return earliest;
}

QDateTime ServerRateLimiter::on429(const QString &host, const QDateTime &now, qint64 retryAfterMs)
{
    QDateTime existing = backoffUntil.value(host);
    int round = retryRounds.value(host, 0);

    if (!existing.isValid() || now >= existing) {
        if (last429.value(host).isValid() && now >= last429.value(host).addMSecs(RESET_GRACE_MS)) {
            round = 0;
        }
        round = qMin(round + 1, MAX_RETRIES);
        retryRounds.insert(host, round);
    }
    last429.insert(host, now);

    qint64 delay;
    if (round >= MAX_RETRIES) {
        delay = EXHAUSTED_BACKOFF_MS;
    } else {
        delay = qMax<qint64>(MIN_429_BACKOFF_MS, retryAfterMs);
        delay = qMin<qint64>(delay, MAX_BACKOFF_MS);
    }

    QDateTime deadline = now.addMSecs(delay);
    if (existing.isValid() && existing > deadline) {
        deadline = existing; // never shorten an active backoff
    }
    backoffUntil.insert(host, deadline);

    return deadline;
}

void ServerRateLimiter::onSuccess(const QString &host)
{
    backoffUntil.remove(host);
    retryRounds.remove(host);
    last429.remove(host);
}

void ServerRateLimiter::clearExpired(const QDateTime &now)
{
    auto it = backoffUntil.begin();
    while (it != backoffUntil.end()) {
        if (now < it.value()) {
            ++it;
            continue;
        }

        QString host = it.key();
        bool budgetStillFresh = last429.value(host).isValid() && now < last429.value(host).addMSecs(RESET_GRACE_MS);
        it = backoffUntil.erase(it);
        if (!budgetStillFresh) {
            retryRounds.remove(host);
            last429.remove(host);
        }
    }
}
