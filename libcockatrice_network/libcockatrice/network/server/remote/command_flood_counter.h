#ifndef COMMAND_FLOOD_COUNTER_H
#define COMMAND_FLOOD_COUNTER_H

#include <QList>
#include <libcockatrice/protocol/pb/game_commands.pb.h>

/**
 * @class CommandFloodCounter
 * @brief Sliding-window counter behind the server's game-command flood limit.
 *
 * The window is a list of per-keep-alive buckets, newest first. advance() opens a
 * bucket and drops the oldest ones that fell out of the window; record() counts a
 * command in the newest bucket unless the command is flood-exempt; exceeds() sums
 * the window. A handler can receive commands before the first keep-alive tick, so
 * record() opens the first bucket itself.
 */
class CommandFloodCounter
{
public:
    /**
     * @brief Opens a new counting interval and trims the window to its size.
     * @param bucketCount Number of intervals the flood limit spans
     */
    void advance(int bucketCount);

    /**
     * @brief Counts one command in the newest interval unless the command is flood-exempt.
     * @param command The command's protocol type
     */
    void record(GameCommand::GameCommandType command);

    /** @return The number of counted commands in the window. */
    [[nodiscard]] int total() const;

    /**
     * @brief Whether the window holds more commands than the limit allows.
     * @param maxCount Commands allowed per window; zero or less disables the limit
     */
    [[nodiscard]] bool exceeds(int maxCount) const;

private:
    /**
     * @brief Whether a command is exempt from the flood limit.
     *
     * Bursty commands stay out of the count, so drawing ten cards one by one or
     * tapping ten cards at once does not trip the limit.
     */
    [[nodiscard]] static bool isFloodExempt(GameCommand::GameCommandType command);

    QList<int> countsPerInterval;
};

#endif // COMMAND_FLOOD_COUNTER_H
