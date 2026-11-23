#include "command_flood_counter.h"

namespace
{
/// Commands that repeat one player action instead of spamming the game, and therefore never count.
const QList<GameCommand::GameCommandType> &floodExemptCommands()
{
    static const QList<GameCommand::GameCommandType> exempt =
        QList<GameCommand::GameCommandType>()
        // draw/undo card draw (example: drawing 10 cards one by one from the deck)
        << GameCommand::DRAW_CARDS
        << GameCommand::UNDO_DRAW
        // create, delete arrows (example: targeting with 10 cards during an attack)
        << GameCommand::CREATE_ARROW
        << GameCommand::DELETE_ARROW
        // set card attributes (example: tapping 10 cards at once)
        << GameCommand::SET_CARD_ATTR
        // increment / decrement counter (example: -10 life points one by one)
        << GameCommand::INC_COUNTER
        // mulling lots of hands in a row
        << GameCommand::MULLIGAN
        // allows a user to sideboard without receiving flooding message
        << GameCommand::MOVE_CARD;
    return exempt;
}
} // namespace

bool CommandFloodCounter::isFloodExempt(GameCommand::GameCommandType command)
{
    return floodExemptCommands().contains(command);
}

void CommandFloodCounter::advance(int bucketCount)
{
    countsPerInterval.prepend(0);
    if (countsPerInterval.size() > bucketCount) {
        countsPerInterval.removeLast();
    }
}

void CommandFloodCounter::record(GameCommand::GameCommandType command)
{
    if (countsPerInterval.isEmpty()) {
        countsPerInterval.prepend(0);
    }

    if (!isFloodExempt(command)) {
        ++countsPerInterval[0];
    }
}

int CommandFloodCounter::total() const
{
    int totalCount = 0;
    for (int count : countsPerInterval) {
        totalCount += count;
    }
    return totalCount;
}

bool CommandFloodCounter::exceeds(int maxCount) const
{
    return maxCount > 0 && total() > maxCount;
}
