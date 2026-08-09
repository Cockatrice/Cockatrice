#include "gtest/gtest.h"
#include <QDateTime>
#include <QTimeZone>
#include <libcockatrice/utility/server_rate_limiter.h>

namespace
{

const QString HOST = "api.scryfall.com";

QDateTime timeAt(qint64 secsFromEpoch)
{
    return QDateTime::fromMSecsSinceEpoch(secsFromEpoch * 1000, QTimeZone::UTC);
}

TEST(ServerRateLimiterTest, First429SetsThirtySecondBackoff)
{
    ServerRateLimiter limiter;
    QDateTime now = timeAt(1000);

    QDateTime deadline = limiter.on429(HOST, now);

    EXPECT_TRUE(limiter.isRateLimited(HOST, now));
    EXPECT_EQ(now.addSecs(30), deadline);
    EXPECT_EQ(1, limiter.rounds(HOST));
    EXPECT_FALSE(limiter.isRateLimited(HOST, deadline));
}

TEST(ServerRateLimiterTest, RetryAfterHeaderIsHonoredWhenLarger)
{
    ServerRateLimiter limiter;
    QDateTime now = timeAt(1000);

    QDateTime deadline = limiter.on429(HOST, now, 45 * 1000);

    EXPECT_EQ(now.addSecs(45), deadline);
}

TEST(ServerRateLimiterTest, RetryAfterSmallerThanFloorIsIgnored)
{
    ServerRateLimiter limiter;
    QDateTime now = timeAt(1000);

    QDateTime deadline = limiter.on429(HOST, now, 5 * 1000);

    EXPECT_EQ(now.addSecs(30), deadline);
}

TEST(ServerRateLimiterTest, Concurrent429sDoNotEscalateOrShorten)
{
    ServerRateLimiter limiter;
    QDateTime now = timeAt(1000);

    QDateTime first = limiter.on429(HOST, now);
    limiter.on429(HOST, now.addMSecs(100));
    limiter.on429(HOST, now.addMSecs(200));

    // The burst counts as a single round, but each 429 slides the deadline forward.
    EXPECT_EQ(1, limiter.rounds(HOST));
    EXPECT_EQ(now.addMSecs(30200), limiter.deadline(HOST));
    EXPECT_TRUE(limiter.deadline(HOST) >= first);
}

TEST(ServerRateLimiterTest, EscalatesAfterWindowPasses)
{
    ServerRateLimiter limiter;
    QDateTime now = timeAt(1000);

    limiter.on429(HOST, now);
    limiter.on429(HOST, now.addSecs(31));

    EXPECT_EQ(2, limiter.rounds(HOST));
}

TEST(ServerRateLimiterTest, BudgetExhaustionAppliesCooldown)
{
    ServerRateLimiter limiter;
    QDateTime now = timeAt(1000);

    QDateTime deadline;
    for (int i = 0; i < ServerRateLimiter::MAX_RETRIES; ++i) {
        deadline = limiter.on429(HOST, now.addSecs(31 * i));
    }

    EXPECT_EQ(ServerRateLimiter::MAX_RETRIES, limiter.rounds(HOST));
    EXPECT_EQ(now.addSecs(31 * (ServerRateLimiter::MAX_RETRIES - 1) + 60), deadline);

    // A 429 inside the exhausted cooldown does not escalate, but keeps sliding the cooldown forward.
    limiter.on429(HOST, now.addSecs(31 * (ServerRateLimiter::MAX_RETRIES - 1) + 30));
    EXPECT_EQ(ServerRateLimiter::MAX_RETRIES, limiter.rounds(HOST));
    EXPECT_EQ(now.addSecs(31 * (ServerRateLimiter::MAX_RETRIES - 1) + 30 + 60), limiter.deadline(HOST));
}

TEST(ServerRateLimiterTest, SuccessClearsPenalty)
{
    ServerRateLimiter limiter;
    QDateTime now = timeAt(1000);

    limiter.on429(HOST, now);
    limiter.onSuccess(HOST);

    EXPECT_EQ(0, limiter.rounds(HOST));
    EXPECT_FALSE(limiter.isRateLimited(HOST, now));
}

TEST(ServerRateLimiterTest, ClearExpiredRefreshesStaleBudget)
{
    ServerRateLimiter limiter;
    QDateTime now = timeAt(1000);

    limiter.on429(HOST, now);
    limiter.clearExpired(now.addSecs(400));

    EXPECT_EQ(0, limiter.rounds(HOST));
    EXPECT_FALSE(limiter.isRateLimited(HOST, now.addSecs(400)));
}

TEST(ServerRateLimiterTest, ClearExpiredKeepsRecentBudget)
{
    ServerRateLimiter limiter;
    QDateTime now = timeAt(1000);

    for (int i = 0; i < ServerRateLimiter::MAX_RETRIES; ++i) {
        limiter.on429(HOST, now.addSecs(31 * i));
    }
    limiter.clearExpired(now.addSecs(200));

    EXPECT_EQ(ServerRateLimiter::MAX_RETRIES, limiter.rounds(HOST));
    EXPECT_FALSE(limiter.isRateLimited(HOST, now.addSecs(200)));
}

TEST(ServerRateLimiterTest, EarliestDeadlineAcrossHosts)
{
    ServerRateLimiter limiter;
    QDateTime now = timeAt(1000);

    limiter.on429("api.scryfall.com", now, 40 * 1000);
    limiter.on429("gatherer.wizards.com", now.addSecs(5));

    EXPECT_EQ(now.addSecs(35), limiter.earliestDeadline(now));
    EXPECT_EQ(now.addSecs(40), limiter.deadline("api.scryfall.com"));
    EXPECT_EQ(now.addSecs(40), limiter.earliestDeadline(now.addSecs(36)));
}

} // namespace

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
