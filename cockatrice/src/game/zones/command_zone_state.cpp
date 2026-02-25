/**
 * @file command_zone_state.cpp
 * @brief Implementation of CommandZoneState pure state machine.
 *
 * @see CommandZoneState for class documentation
 * @see CommandZone for the graphics layer that owns this state
 */

#include "command_zone_state.h"

CommandZoneState::CommandZoneState(qreal zoneHeight, CommandZoneType zoneType)
    : m_zoneHeight(zoneHeight), m_zoneType(zoneType),
      m_visibility(zoneType == CommandZoneType::Primary ? ZoneVisibility::Expanded : ZoneVisibility::Collapsed)
{
}

bool CommandZoneState::isPrimary() const
{
    return m_zoneType == CommandZoneType::Primary;
}

bool CommandZoneState::isMinimized() const
{
    return m_visibility == ZoneVisibility::Minimized;
}

bool CommandZoneState::isExpanded() const
{
    return m_visibility != ZoneVisibility::Collapsed;
}

bool CommandZoneState::isCollapsed() const
{
    return m_visibility == ZoneVisibility::Collapsed;
}

qreal CommandZoneState::currentHeight() const
{
    if (isCollapsed()) {
        return 0;
    }
    return isMinimized() ? (m_zoneHeight * CommandZoneConstants::MINIMIZED_HEIGHT_RATIO) : m_zoneHeight;
}

CommandZoneType CommandZoneState::getZoneType() const
{
    return m_zoneType;
}

qreal CommandZoneState::getZoneHeight() const
{
    return m_zoneHeight;
}

StateChangeResult CommandZoneState::trySetExpanded(bool expanded, bool collapseBlocked)
{
    if (isPrimary()) {
        return StateChangeResult::noChange();
    }

    bool currentlyExpanded = isExpanded();
    if (currentlyExpanded == expanded) {
        return StateChangeResult::noChange();
    }

    if (!expanded && collapseBlocked) {
        return StateChangeResult::noChange();
    }

    bool wasMinimized = isMinimized();

    if (expanded) {
        m_visibility = ZoneVisibility::Expanded;
    } else {
        m_visibility = ZoneVisibility::Collapsed;
    }

    return {
        true,                     // geometryChanged
        true,                     // shouldEmitExpanded
        wasMinimized && !expanded // shouldEmitMinimized (only if we were minimized and collapsed)
    };
}

StateChangeResult CommandZoneState::tryToggleExpanded(bool collapseBlocked)
{
    if (isPrimary()) {
        return StateChangeResult::noChange();
    }

    if (m_lastToggle.isValid() && m_lastToggle.elapsed() < TOGGLE_DEBOUNCE_MS) {
        return StateChangeResult::noChange();
    }

    if (isExpanded() && collapseBlocked) {
        return StateChangeResult::noChange();
    }

    m_lastToggle.start();
    return trySetExpanded(!isExpanded(), collapseBlocked);
}

StateChangeResult CommandZoneState::tryToggleMinimized()
{
    if (isCollapsed()) {
        return StateChangeResult::noChange();
    }

    m_visibility = isMinimized() ? ZoneVisibility::Expanded : ZoneVisibility::Minimized;

    return {
        true,  // geometryChanged
        false, // shouldEmitExpanded (minimized toggle doesn't change expanded state)
        true   // shouldEmitMinimized
    };
}

void CommandZoneState::resetDebounce()
{
    m_lastToggle.invalidate();
}
