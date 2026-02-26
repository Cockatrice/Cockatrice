/**
 * @file command_zone_types.h
 * @ingroup GameGraphicsZones
 * @brief Type definitions for command zone state management.
 *
 * This header is intentionally minimal and dependency-free so it can be
 * included in both production code and isolated unit tests.
 */

#ifndef COCKATRICE_COMMAND_ZONE_TYPES_H
#define COCKATRICE_COMMAND_ZONE_TYPES_H

/**
 * @enum CommandZoneType
 * @brief Identifies the type of command zone.
 *
 * Each zone type has different behavior:
 * - Primary: Always visible, never collapsible, has tax counter
 * - Partner: Collapsible, has tax counter
 * - Companion: Collapsible, no tax counter
 * - Background: Collapsible, no tax counter
 */
enum class CommandZoneType
{
    Primary,   ///< Main commander - always visible, never collapsible
    Partner,   ///< Partner commander
    Companion, ///< Companion
    Background ///< Background
};

/**
 * @enum ZoneVisibility
 * @brief Represents the visual display state of a zone.
 */
enum class ZoneVisibility
{
    Expanded,  ///< Full height, cards visible
    Minimized, ///< 25% height, cards clipped
    Collapsed  ///< Zero height, only toggle button visible (not valid for Primary)
};

/**
 * @namespace CommandZoneConstants
 * @brief Constants for command zone sizing.
 */
namespace CommandZoneConstants
{
constexpr double MINIMIZED_HEIGHT_RATIO = 0.25;
}

/**
 * @brief Returns whether the given zone type supports multiple cards.
 *
 * Primary and Partner zones hold exactly one card (commander rules,
 * tax counter positioning). Companion and Background zones support
 * multiple cards to handle rare mechanics like multiple companions.
 *
 * @param type The command zone type to check
 * @return true if the zone can hold multiple cards, false for single-card zones
 */
inline bool supportsMultipleCards(CommandZoneType type)
{
    return type == CommandZoneType::Companion || type == CommandZoneType::Background;
}

/**
 * @struct StateChangeResult
 * @brief Result of a state mutation. Use the boolean flags to determine side effects;
 * do not interpret visibility transitions directly (information hiding principle).
 */
struct StateChangeResult
{
    bool geometryChanged;     ///< Caller should call prepareGeometryChange() and update layout
    bool shouldEmitExpanded;  ///< Caller should emit expandedChanged(isExpanded())
    bool shouldEmitMinimized; ///< Caller should emit minimizedChanged(isMinimized())

    /**
     * @brief Returns a result indicating no changes occurred.
     * @return StateChangeResult with all flags false
     */
    static StateChangeResult noChange()
    {
        return {false, false, false};
    }
};

#endif // COCKATRICE_COMMAND_ZONE_TYPES_H
