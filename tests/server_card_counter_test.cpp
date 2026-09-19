/** @file server_card_counter_test.cpp
 *  @brief Tests for Server_Card counter operations.
 *  @ingroup Tests
 */

#include <gtest/gtest.h>
#include <libcockatrice/network/server/remote/game/server_card.h>
#include <libcockatrice/protocol/pb/event_set_card_counter.pb.h>
#include <libcockatrice/utility/card_ref.h>
#include <libcockatrice/utility/counter_limits.h>
#include <limits>

TEST(ServerCardCounter, IncrementNewCounter)
{
    Server_Card card(CardRef{"TestCard", ""}, 1, 0, 0);
    EXPECT_TRUE(card.incrementCounter(1, 10));
    EXPECT_EQ(card.getCounter(1), 10);
}

TEST(ServerCardCounter, IncrementExistingCounter)
{
    Server_Card card(CardRef{"TestCard", ""}, 1, 0, 0);
    ASSERT_TRUE(card.setCounter(1, 50));
    EXPECT_TRUE(card.incrementCounter(1, 10));
    EXPECT_EQ(card.getCounter(1), 60);
}

TEST(ServerCardCounter, IncrementOverflowProtection)
{
    Server_Card card(CardRef{"TestCard", ""}, 1, 0, 0);
    ASSERT_TRUE(card.setCounter(1, MAX_COUNTER_VALUE));
    EXPECT_FALSE(card.incrementCounter(1, 1));
    EXPECT_EQ(card.getCounter(1), MAX_COUNTER_VALUE);
}

TEST(ServerCardCounter, DecrementUnderflowProtection)
{
    Server_Card card(CardRef{"TestCard", ""}, 1, 0, 0);
    ASSERT_TRUE(card.setCounter(1, 5));
    EXPECT_TRUE(card.incrementCounter(1, -10));
    EXPECT_EQ(card.getCounter(1), 0);
    EXPECT_FALSE(card.getCounters().contains(1));
}

TEST(ServerCardCounter, ReturnsFalseWhenUnchanged)
{
    Server_Card card(CardRef{"TestCard", ""}, 1, 0, 0);
    ASSERT_TRUE(card.setCounter(1, 50));
    EXPECT_FALSE(card.incrementCounter(1, 0));
    EXPECT_EQ(card.getCounter(1), 50);
}

TEST(ServerCardCounter, DecrementToZeroRemovesCounter)
{
    Server_Card card(CardRef{"TestCard", ""}, 1, 0, 0);
    ASSERT_TRUE(card.setCounter(1, 10));
    EXPECT_TRUE(card.incrementCounter(1, -10));
    EXPECT_EQ(card.getCounter(1), 0);
    EXPECT_FALSE(card.getCounters().contains(1));
}

TEST(ServerCardCounter, SetToZeroRemovesCounter)
{
    Server_Card card(CardRef{"TestCard", ""}, 1, 0, 0);
    ASSERT_TRUE(card.setCounter(1, 10));
    EXPECT_TRUE(card.setCounter(1, 0));
    EXPECT_EQ(card.getCounter(1), 0);
    EXPECT_FALSE(card.getCounters().contains(1));
}

TEST(ServerCardCounter, SetCounterReturnsFalseWhenUnchanged)
{
    Server_Card card(CardRef{"TestCard", ""}, 1, 0, 0);
    ASSERT_TRUE(card.setCounter(1, 50));
    EXPECT_FALSE(card.setCounter(1, 50));
    EXPECT_EQ(card.getCounter(1), 50);
}

TEST(ServerCardCounter, SetCounterReturnsTrueWhenChanged)
{
    Server_Card card(CardRef{"TestCard", ""}, 1, 0, 0);
    ASSERT_TRUE(card.setCounter(1, 50));
    EXPECT_TRUE(card.setCounter(1, 100));
    EXPECT_EQ(card.getCounter(1), 100);
}

TEST(ServerCardCounter, SetCounterEventNotPopulatedWhenUnchanged)
{
    Server_Card card(CardRef{"TestCard", ""}, 1, 0, 0);
    ASSERT_TRUE(card.setCounter(1, 50));

    Event_SetCardCounter event;
    event.set_counter_id(999);
    event.set_counter_value(999);

    EXPECT_FALSE(card.setCounter(1, 50, &event));
    EXPECT_EQ(event.counter_id(), 999);
    EXPECT_EQ(event.counter_value(), 999);
}

TEST(ServerCardCounter, IncrementCounterPopulatesEvent)
{
    Server_Card card(CardRef{"TestCard", ""}, 1, 0, 0);
    ASSERT_TRUE(card.setCounter(1, 50));

    Event_SetCardCounter event;
    EXPECT_TRUE(card.incrementCounter(1, 10, &event));

    EXPECT_EQ(event.counter_id(), 1);
    EXPECT_EQ(event.counter_value(), 60);
}

TEST(ServerCardCounter, IncrementCounterEventReflectsClampedValue)
{
    Server_Card card(CardRef{"TestCard", ""}, 1, 0, 0);
    ASSERT_TRUE(card.setCounter(1, MAX_COUNTER_VALUE - 5));

    Event_SetCardCounter event;
    EXPECT_TRUE(card.incrementCounter(1, 10, &event));

    EXPECT_EQ(event.counter_id(), 1);
    EXPECT_EQ(event.counter_value(), MAX_COUNTER_VALUE);
}

TEST(ServerCardCounter, IncrementCounterNoEventWhenNullptr)
{
    Server_Card card(CardRef{"TestCard", ""}, 1, 0, 0);
    ASSERT_TRUE(card.setCounter(1, 50));
    EXPECT_TRUE(card.incrementCounter(1, 10, nullptr));
    EXPECT_EQ(card.getCounter(1), 60);
}

TEST(ServerCardCounter, IncrementCounterEventNotPopulatedWhenUnchanged)
{
    Server_Card card(CardRef{"TestCard", ""}, 1, 0, 0);
    ASSERT_TRUE(card.setCounter(1, MAX_COUNTER_VALUE));

    Event_SetCardCounter event;
    event.set_counter_id(999);
    event.set_counter_value(999);

    EXPECT_FALSE(card.incrementCounter(1, 1, &event));
    EXPECT_EQ(event.counter_id(), 999);
    EXPECT_EQ(event.counter_value(), 999);
}

TEST(ServerCardCounter, SetCounterClampsNegativeToZero)
{
    Server_Card card(CardRef{"TestCard", ""}, 1, 0, 0);
    EXPECT_FALSE(card.setCounter(1, -5));
    EXPECT_EQ(card.getCounter(1), 0);
    EXPECT_FALSE(card.getCounters().contains(1));
}

TEST(ServerCardCounter, SetCounterClampsAboveMaxToMax)
{
    Server_Card card(CardRef{"TestCard", ""}, 1, 0, 0);
    EXPECT_TRUE(card.setCounter(1, 1500));
    EXPECT_EQ(card.getCounter(1), MAX_COUNTER_VALUE);
}

TEST(ServerCardCounter, IncrementDoesNotGoBelowZero)
{
    Server_Card card(CardRef{"TestCard", ""}, 1, 0, 0);
    ASSERT_TRUE(card.setCounter(1, 5));
    EXPECT_TRUE(card.incrementCounter(1, -10));
    EXPECT_EQ(card.getCounter(1), 0);
    EXPECT_FALSE(card.getCounters().contains(1));
}

TEST(ServerCardCounter, IncrementDoesNotExceedMax)
{
    Server_Card card(CardRef{"TestCard", ""}, 1, 0, 0);
    ASSERT_TRUE(card.setCounter(1, MAX_COUNTER_VALUE - 5));
    EXPECT_TRUE(card.incrementCounter(1, 10));
    EXPECT_EQ(card.getCounter(1), MAX_COUNTER_VALUE);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
