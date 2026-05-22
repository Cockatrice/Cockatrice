/** @file server_counter_test.cpp
 *  @brief Tests for Server_Counter operations.
 *  @ingroup Tests
 */

#include <gtest/gtest.h>
#include <libcockatrice/network/server/remote/game/server_counter.h>
#include <libcockatrice/utility/trice_limits.h>
#include <limits>

TEST(ServerCounter, IncrementDoesNotOverflow)
{
    Server_Counter c(1, "test", color(), 10, std::numeric_limits<int>::max());
    bool changed = c.incrementCount(1);
    EXPECT_FALSE(changed);
    EXPECT_EQ(c.getCount(), std::numeric_limits<int>::max());
}

TEST(ServerCounter, DecrementDoesNotUnderflow)
{
    Server_Counter c(1, "test", color(), 10, std::numeric_limits<int>::min());
    bool changed = c.incrementCount(-1);
    EXPECT_FALSE(changed);
    EXPECT_EQ(c.getCount(), std::numeric_limits<int>::min());
}

TEST(ServerCounter, SetCountReturnsFalseWhenUnchanged)
{
    Server_Counter c(1, "test", color(), 10, 50);
    bool changed = c.setCount(50);
    EXPECT_FALSE(changed);
}

TEST(ServerCounter, IncrementReturnsChangeStatus)
{
    Server_Counter c(1, "test", color(), 10, 50);
    EXPECT_TRUE(c.incrementCount(10));
    EXPECT_EQ(c.getCount(), 60);
    EXPECT_FALSE(c.incrementCount(0));
    EXPECT_EQ(c.getCount(), 60);
}

TEST(ServerCounter, LargePositiveDeltaDoesNotOverflow)
{
    Server_Counter c(1, "test", color(), 10, std::numeric_limits<int>::max() - 10);
    bool changed = c.incrementCount(std::numeric_limits<int>::max());
    EXPECT_TRUE(changed); // Value changes from INT_MAX-10 to INT_MAX (clamped)
    EXPECT_EQ(c.getCount(), std::numeric_limits<int>::max());
}

TEST(ServerCounter, LargeNegativeDeltaDoesNotUnderflow)
{
    Server_Counter c(1, "test", color(), 10, std::numeric_limits<int>::min() + 10);
    bool changed = c.incrementCount(std::numeric_limits<int>::min());
    EXPECT_TRUE(changed); // Value changes from INT_MIN+10 to INT_MIN (clamped)
    EXPECT_EQ(c.getCount(), std::numeric_limits<int>::min());
}

TEST(ServerCounter, SetCountReturnsTrueWhenChanged)
{
    Server_Counter c(1, "test", color(), 10, 50);
    EXPECT_TRUE(c.setCount(100));
    EXPECT_EQ(c.getCount(), 100);
}

TEST(ServerCounter, BasicIncrementWorks)
{
    Server_Counter c(1, "test", color(), 10, 50);
    EXPECT_TRUE(c.incrementCount(10));
    EXPECT_EQ(c.getCount(), 60);
    EXPECT_TRUE(c.incrementCount(-20));
    EXPECT_EQ(c.getCount(), 40);
}

TEST(ServerCounter, MixedExtremesDoNotClamp)
{
    Server_Counter c(1, "test", color(), 10, std::numeric_limits<int>::max());
    bool changed = c.incrementCount(std::numeric_limits<int>::min());
    EXPECT_TRUE(changed);
    EXPECT_EQ(c.getCount(), -1);
}

TEST(ServerCounter, SetCountClampsToCustomBounds)
{
    Server_Counter c(1, "test", color(), 10, 50, 0, 100);
    EXPECT_TRUE(c.setCount(150));
    EXPECT_EQ(c.getCount(), 100);
    EXPECT_TRUE(c.setCount(-10));
    EXPECT_EQ(c.getCount(), 0);
}

TEST(ServerCounter, IncrementClampsToCustomBounds)
{
    Server_Counter c(1, "test", color(), 10, 50, 0, 100);
    EXPECT_TRUE(c.incrementCount(100));
    EXPECT_EQ(c.getCount(), 100);
    EXPECT_FALSE(c.incrementCount(1));
    EXPECT_EQ(c.getCount(), 100);
    EXPECT_TRUE(c.incrementCount(-200));
    EXPECT_EQ(c.getCount(), 0);
    EXPECT_FALSE(c.incrementCount(-1));
    EXPECT_EQ(c.getCount(), 0);
}

TEST(ServerCounter, CustomBoundsForCommanderTax)
{
    Server_Counter taxCounter(1, "tax", color(), 20, 0, 0, MAX_COUNTER_VALUE);
    EXPECT_TRUE(taxCounter.setCount(1000));
    EXPECT_EQ(taxCounter.getCount(), MAX_COUNTER_VALUE);
    EXPECT_TRUE(taxCounter.setCount(-5));
    EXPECT_EQ(taxCounter.getCount(), 0);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
