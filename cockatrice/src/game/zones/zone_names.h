/**
 * @file zone_names.h
 * @ingroup GameLogicZones
 * @brief Canonical string identifiers for game zones.
 */

#ifndef ZONE_NAMES_H
#define ZONE_NAMES_H

/**
 * @namespace ZoneNames
 * @brief String constants for zone identification in protocol and lookups.
 *
 * These constants define the canonical names used to identify zones
 * in network protocol messages and zone lookup operations.
 */
namespace ZoneNames
{
constexpr const char *TABLE = "table";
constexpr const char *GRAVE = "grave";
constexpr const char *EXILE = "rfg";
constexpr const char *HAND = "hand";
constexpr const char *DECK = "deck";
constexpr const char *SIDEBOARD = "sb";
constexpr const char *STACK = "stack";
constexpr const char *COMMAND = "command";
constexpr const char *PARTNER = "partner";
constexpr const char *COMPANION = "companion";
constexpr const char *BACKGROUND = "background";
} // namespace ZoneNames

#endif // ZONE_NAMES_H
