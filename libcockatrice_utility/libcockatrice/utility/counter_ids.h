/**
 * @file counter_ids.h
 * @ingroup GameLogic
 * @brief Shared counter IDs and names for system counters (e.g. commander tax).
 */

#ifndef COCKATRICE_COUNTER_IDS_H
#define COCKATRICE_COUNTER_IDS_H

#include <QString>

/**
 * Shared counter IDs used by both client and server.
 * These must match between server_player.cpp and player_event_handler.cpp.
 *
 * Reserved counter IDs for system counters:
 *   IDs 0-7:  Standard player counters (life, mana colors, storm)
 *   IDs 8-9:  Commander tax counters
 *   IDs 10+:  Available for user-created counters (FirstUserId)
 *
 * newCounterId() always returns >= FirstUserId to prevent user counters
 * from colliding with reserved IDs, even in non-Commander games.
 */
namespace CounterIds
{
// Tax counters store a cast count (times cast from command zone).
constexpr int CommanderTax = 8; ///< Primary commander tax counter
constexpr int PartnerTax = 9;   ///< Partner commander tax counter
constexpr int FirstUserId = 10; ///< First ID available for user-created counters

inline bool isTaxCounter(int id)
{
    return id == CommanderTax || id == PartnerTax;
}
} // namespace CounterIds

namespace CounterNames
{
constexpr const char *CommanderTax = "commander_tax_counter";
constexpr const char *PartnerTax = "partner_tax_counter";

inline bool isTaxCounter(const QString &name)
{
    return name == CommanderTax || name == PartnerTax;
}
} // namespace CounterNames

#endif // COCKATRICE_COUNTER_IDS_H
