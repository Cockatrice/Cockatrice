/** @file command_flood_counter_test.cpp
 *  @brief Tests for CommandFloodCounter.
 *  @ingroup Tests
 */

#include <gtest/gtest.h>
#include <libcockatrice/network/server/remote/command_flood_counter.h>

namespace
{
constexpr auto COUNTED_COMMAND = GameCommand::CONCEDE;

const QList<GameCommand::GameCommandType> floodExemptCommands = {
    GameCommand::DRAW_CARDS,    GameCommand::UNDO_DRAW,   GameCommand::CREATE_ARROW, GameCommand::DELETE_ARROW,
    GameCommand::SET_CARD_ATTR, GameCommand::INC_COUNTER, GameCommand::MULLIGAN,     GameCommand::MOVE_CARD};
} // namespace

TEST(CommandFloodCounterTest, CountedCommandsAccumulateInTheWindow)
{
    CommandFloodCounter counter;

    counter.record(COUNTED_COMMAND);
    counter.record(COUNTED_COMMAND);

    EXPECT_EQ(2, counter.total());
}

TEST(CommandFloodCounterTest, FloodExemptCommandsAreNotCounted)
{
    CommandFloodCounter counter;

    for (auto command : floodExemptCommands) {
        counter.record(command);
    }

    EXPECT_EQ(0, counter.total());
}

TEST(CommandFloodCounterTest, ExceedsComparesTheWindowAgainstTheLimit)
{
    CommandFloodCounter counter;

    counter.record(COUNTED_COMMAND);
    counter.record(COUNTED_COMMAND);
    counter.record(COUNTED_COMMAND);

    EXPECT_TRUE(counter.exceeds(2));
    EXPECT_FALSE(counter.exceeds(3));
}

TEST(CommandFloodCounterTest, NonPositiveLimitDisablesTheCheck)
{
    CommandFloodCounter counter;

    counter.record(COUNTED_COMMAND);
    counter.record(COUNTED_COMMAND);

    EXPECT_FALSE(counter.exceeds(0));
    EXPECT_FALSE(counter.exceeds(-1));
}

TEST(CommandFloodCounterTest, FirstCommandOpensTheIntervalWithoutAnAdvance)
{
    CommandFloodCounter counter;

    counter.record(COUNTED_COMMAND);

    EXPECT_EQ(1, counter.total());
}

TEST(CommandFloodCounterTest, AdvanceOpensABucketAndDropsTheOldest)
{
    CommandFloodCounter counter;

    counter.record(COUNTED_COMMAND);
    counter.record(COUNTED_COMMAND);
    counter.advance(2);
    EXPECT_EQ(2, counter.total());

    counter.record(COUNTED_COMMAND);
    EXPECT_EQ(3, counter.total());

    counter.advance(2);
    EXPECT_EQ(1, counter.total());
}

TEST(CommandFloodCounterTest, AdvanceWithARatioBelowOneClearsTheWindow)
{
    CommandFloodCounter counter;

    counter.record(COUNTED_COMMAND);
    counter.advance(0);

    EXPECT_EQ(0, counter.total());
}
