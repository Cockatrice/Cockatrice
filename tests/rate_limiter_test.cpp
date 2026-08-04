#include "ratelimiter.h"

#include "gtest/gtest.h"

namespace
{

TEST(RateLimiterTest, AllowsAttemptsWithinLimit)
{
    RateLimiter limiter;
    ASSERT_FALSE(limiter.recordAttempt("ip", 3, 60));
    ASSERT_FALSE(limiter.recordAttempt("ip", 3, 60));
    ASSERT_FALSE(limiter.recordAttempt("ip", 3, 60));
    ASSERT_FALSE(limiter.isBlocked("ip", 3, 60));
}

TEST(RateLimiterTest, BlocksAttemptsOverLimit)
{
    RateLimiter limiter;
    ASSERT_FALSE(limiter.recordAttempt("ip", 2, 60));
    ASSERT_FALSE(limiter.recordAttempt("ip", 2, 60));
    ASSERT_TRUE(limiter.recordAttempt("ip", 2, 60));
    ASSERT_TRUE(limiter.isBlocked("ip", 2, 60));
}

TEST(RateLimiterTest, ClearAttempts)
{
    RateLimiter limiter;
    ASSERT_TRUE(limiter.recordAttempt("ip", 0, 60));
    ASSERT_TRUE(limiter.isBlocked("ip", 0, 60));
    limiter.clearAttempts("ip");
    ASSERT_FALSE(limiter.isBlocked("ip", 0, 60));
}

TEST(RateLimiterTest, KeysAreIndependent)
{
    RateLimiter limiter;
    ASSERT_TRUE(limiter.recordAttempt("a", 0, 60));
    ASSERT_FALSE(limiter.isBlocked("b", 0, 60));
    ASSERT_TRUE(limiter.isBlocked("a", 0, 60));
}

} // namespace

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
