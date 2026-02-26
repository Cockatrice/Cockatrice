#ifndef COUNTER_IDS_H
#define COUNTER_IDS_H

/**
 * Shared counter IDs used by both client and server.
 * These must match between server_player.cpp and player_event_handler.cpp.
 *
 * Reserved counter IDs for system counters:
 *   IDs 0-7:  Standard player counters (life, mana colors, storm)
 *   IDs 8-9:  Commander tax counters (reserved, do not use for custom counters)
 *   IDs 10+:  Available for user-created counters
 *
 * The server's newCounterId() starts from the highest existing ID + 1,
 * so these reserved IDs won't conflict as long as they're created first
 * during setupZones().
 */
namespace CounterIds
{
constexpr int CommanderTax = 8;
constexpr int PartnerTax = 9;
} // namespace CounterIds

namespace CounterNames
{
constexpr const char *CommanderTax = "commander_tax_counter";
constexpr const char *PartnerTax = "partner_tax_counter";
} // namespace CounterNames

#endif // COUNTER_IDS_H
