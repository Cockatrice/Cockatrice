/**
 * @file abstract_player_component.h
 * @ingroup GameMenusPlayers
 * @brief Polymorphic interface for player-bound UI components managed by PlayerMenu.
 */

#ifndef COCKATRICE_ABSTRACT_PLAYER_COMPONENT_H
#define COCKATRICE_ABSTRACT_PLAYER_COMPONENT_H

/**
 * @brief Interface for player-bound UI components that need shortcut and translation lifecycle management.
 *
 * Not a QObject — avoids diamond inheritance with Qt's MOC. Each concrete component
 * inherits QObject through its Qt base class (QMenu, TearOffMenu, QGraphicsItem, etc.)
 * and this interface through regular multiple inheritance.
 */
class AbstractPlayerComponent
{
public:
    virtual ~AbstractPlayerComponent() = default;

    /// Bind keyboard shortcuts. Called when this player gains focus.
    virtual void setShortcutsActive() = 0;

    /// Unbind keyboard shortcuts. Called when this player loses focus.
    virtual void setShortcutsInactive() = 0;

    /// Retranslate all user-visible strings. Called on language change.
    virtual void retranslateUi() = 0;
};

#endif // COCKATRICE_ABSTRACT_PLAYER_COMPONENT_H
