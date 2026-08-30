#include "ratelimiter.h"

#include "gtest/gtest.h"

namespace
{

TEST(RateLimiterTest, AllowsAttemptsWithinLimit)
{
    RateLimiter limiter;
    ASSERT_FALSE(limiter.recordAttemptAt("ip", 3, 60, 1000));
    ASSERT_FALSE(limiter.recordAttemptAt("ip", 3, 60, 1001));
    // Still under the limit after two attempts, so the key is not blocked...
    ASSERT_FALSE(limiter.isBlockedAt("ip", 3, 60, 1002));
    // ...and the third (the limit) is allowed through.
    ASSERT_FALSE(limiter.recordAttemptAt("ip", 3, 60, 1002));
    // The next attempt is blocked once three are on the books.
    ASSERT_TRUE(limiter.isBlockedAt("ip", 3, 60, 1003));
    ASSERT_TRUE(limiter.recordAttemptAt("ip", 3, 60, 1003));
}

TEST(RateLimiterTest, BlocksAttemptsAtTheLimit)
{
    RateLimiter limiter;
    ASSERT_FALSE(limiter.recordAttemptAt("ip", 2, 60, 1000));
    ASSERT_FALSE(limiter.recordAttemptAt("ip", 2, 60, 1001));
    ASSERT_TRUE(limiter.recordAttemptAt("ip", 2, 60, 1002));
    ASSERT_TRUE(limiter.isBlockedAt("ip", 2, 60, 1002));
}

TEST(RateLimiterTest, NonPositiveConfigDisablesLimiting)
{
    RateLimiter limiter;
    // maxAttempts = 0 is the documented "set to 0 to disable"; negative or
    // zero windows are config mistakes and must not lock everyone out either.
    ASSERT_FALSE(limiter.recordAttempt("ip", 0, 60));
    ASSERT_FALSE(limiter.isBlocked("ip", 0, 60));
    ASSERT_FALSE(limiter.recordAttempt("ip", -1, 60));
    ASSERT_FALSE(limiter.isBlocked("ip", -1, 60));
    ASSERT_FALSE(limiter.recordAttempt("ip", 5, 0));
    ASSERT_FALSE(limiter.isBlocked("ip", 5, 0));
}

TEST(RateLimiterTest, ClearAttempts)
{
    RateLimiter limiter;
    ASSERT_FALSE(limiter.recordAttemptAt("ip", 1, 60, 1000));
    ASSERT_TRUE(limiter.recordAttemptAt("ip", 1, 60, 1001));
    ASSERT_TRUE(limiter.isBlockedAt("ip", 1, 60, 1001));
    limiter.clearAttempts("ip");
    ASSERT_FALSE(limiter.isBlockedAt("ip", 1, 60, 1002));
    // The cleared key starts fresh and is allowed again.
    ASSERT_FALSE(limiter.recordAttemptAt("ip", 1, 60, 1002));
}

TEST(RateLimiterTest, KeysAreIndependent)
{
    RateLimiter limiter;
    ASSERT_FALSE(limiter.recordAttemptAt("a", 1, 60, 1000));
    ASSERT_TRUE(limiter.recordAttemptAt("a", 1, 60, 1001));
    ASSERT_FALSE(limiter.isBlockedAt("b", 1, 60, 1001));
    ASSERT_TRUE(limiter.isBlockedAt("a", 1, 60, 1001));
}

TEST(RateLimiterTest, AttemptsAgeOutOfTheWindow)
{
    RateLimiter limiter;
    const qint64 t = 1000;
    const int window = 60;

    ASSERT_FALSE(limiter.recordAttemptAt("ip", 2, window, t));
    ASSERT_FALSE(limiter.recordAttemptAt("ip", 2, window, t + 1));
    ASSERT_TRUE(limiter.recordAttemptAt("ip", 2, window, t + 2));
    ASSERT_TRUE(limiter.isBlockedAt("ip", 2, window, t + 2));

    // A blocked attempt is rejected without recording, so it cannot extend
    // the lockout either.
    ASSERT_TRUE(limiter.recordAttemptAt("ip", 2, window, t + 3));

    // Once the recorded attempts fall outside the window the key clears by
    // itself, with no manual unlock.
    ASSERT_FALSE(limiter.isBlockedAt("ip", 2, window, t + 61));
    ASSERT_FALSE(limiter.recordAttemptAt("ip", 2, window, t + 62));
}

TEST(RateLimiterTest, BlockedKeyIsUnlockedAfterSuccessfulClear)
{
    RateLimiter limiter;
    const qint64 t = 5000;

    ASSERT_FALSE(limiter.recordAttemptAt("ip", 2, 60, t));
    ASSERT_FALSE(limiter.recordAttemptAt("ip", 2, 60, t + 1));
    ASSERT_TRUE(limiter.recordAttemptAt("ip", 2, 60, t + 2));
    ASSERT_TRUE(limiter.isBlockedAt("ip", 2, 60, t + 2));

    limiter.clearAttempts("ip");
    ASSERT_FALSE(limiter.recordAttemptAt("ip", 2, 60, t + 3));
}

} // namespace

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}