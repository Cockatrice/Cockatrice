/**
 * @file command_zone_state.h
 * @ingroup GameGraphicsZones
 * @brief Pure state machine for command zone visibility management.
 */

#ifndef COCKATRICE_COMMAND_ZONE_STATE_H
#define COCKATRICE_COMMAND_ZONE_STATE_H

#include "command_zone_types.h"

#include <QElapsedTimer>

/**
 * @class CommandZoneState
 * @brief Manages visibility state (expanded/minimized/collapsed) for command zones.
 *
 * This class extracts pure state logic from CommandZone for testability.
 * It has no Qt graphics dependencies and can be tested in isolation.
 *
 * The state machine supports three visibility states:
 * - **Expanded**: Full height, cards visible
 * - **Minimized**: 25% height, cards clipped
 * - **Collapsed**: Zero height, only toggle button visible (not valid for Primary)
 *
 * All mutation methods return a StateChangeResult that tells callers what
 * side effects to trigger (geometry changes, signal emissions), following
 * the information hiding principle.
 *
 * @see CommandZone for the graphics layer that owns this state
 * @see CommandZoneLogic for card data management
 * @see StateChangeResult for mutation result semantics
 */
class CommandZoneState
{
public:
    /**
     * @brief Constructs a CommandZoneState with the given height and type.
     * @param zoneHeight Full height in pixels when expanded
     * @param zoneType Type of command zone (Primary, Partner, Companion, Background)
     *
     * Primary zones start Expanded; all other types start Collapsed.
     */
    CommandZoneState(qreal zoneHeight, CommandZoneType zoneType);

    // === Query Methods ===

    [[nodiscard]] bool isPrimary() const;
    [[nodiscard]] bool isMinimized() const;

    /**
     * @brief Returns whether the zone is currently expanded (not collapsed).
     *
     * Note: A minimized zone is still considered "expanded" because it has
     * non-zero height. This method returns false only for Collapsed visibility.
     */
    [[nodiscard]] bool isExpanded() const;

    [[nodiscard]] bool isCollapsed() const;

    /// Returns current visual height: 0 (collapsed), 25% (minimized), or full.
    [[nodiscard]] qreal currentHeight() const;

    /**
     * @brief Returns the zone type.
     * @return The CommandZoneType of this zone
     */
    [[nodiscard]] CommandZoneType getZoneType() const;

    /**
     * @brief Returns the full (non-minimized) height of the zone.
     * @return Full height in pixels
     */
    [[nodiscard]] qreal getZoneHeight() const;

    // === Mutation Methods ===

    /**
     * @brief Attempts to set the expanded state of a collapsible zone.
     *
     * No-op for Primary zone. When collapsing, transitions from Expanded/Minimized
     * to Collapsed (emitting minimizedChanged if was minimized). When expanding,
     * transitions to Expanded.
     *
     * @param expanded True to expand, false to collapse
     * @param collapseBlocked True if external conditions prevent collapsing (e.g., cards present)
     * @return StateChangeResult indicating what side effects to trigger
     */
    StateChangeResult trySetExpanded(bool expanded, bool collapseBlocked = false);

    /**
     * @brief Attempts to toggle the expanded/collapsed state.
     *
     * No-op for Primary zone. Subject to debounce protection (200ms).
     *
     * @param collapseBlocked True if external conditions prevent collapsing
     * @return StateChangeResult indicating what side effects to trigger
     */
    StateChangeResult tryToggleExpanded(bool collapseBlocked = false);

    /**
     * @brief Attempts to toggle the minimized state.
     *
     * No-op for collapsed zones. Toggles between Expanded and Minimized.
     *
     * @return StateChangeResult indicating what side effects to trigger
     */
    StateChangeResult tryToggleMinimized();

    // === Test Support ===

    /**
     * @brief Resets the debounce timer to allow immediate toggle.
     *
     * @warning For testing only. Do not call in production code.
     */
    void resetDebounce();

private:
    qreal m_zoneHeight;          ///< Full height in pixels when expanded
    CommandZoneType m_zoneType;  ///< Type of command zone
    ZoneVisibility m_visibility; ///< Current visibility state
    QElapsedTimer m_lastToggle;  ///< Debounce timer for toggle operations

    static constexpr qint64 TOGGLE_DEBOUNCE_MS = 200;
};

#endif // COCKATRICE_COMMAND_ZONE_STATE_H
