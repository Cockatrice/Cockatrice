#ifndef ZONE_NAMES_H
#define ZONE_NAMES_H

namespace ZoneNames
{
// Protocol-level zone identifiers shared between client and server.
// These must match exactly across all components.

constexpr const char *TABLE = "table";
constexpr const char *GRAVE = "grave";
constexpr const char *EXILE = "rfg"; // "removed from game"
constexpr const char *HAND = "hand";
constexpr const char *DECK = "deck";
constexpr const char *SIDEBOARD = "sb";
constexpr const char *STACK = "stack";

} // namespace ZoneNames

#endif // ZONE_NAMES_H
