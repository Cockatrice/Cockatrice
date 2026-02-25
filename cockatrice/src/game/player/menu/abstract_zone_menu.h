/**
 * @file abstract_zone_menu.h
 * @ingroup GameMenusZones
 * @brief Abstract interface for zone menus with common shortcut management.
 *
 * This interface enables polymorphic iteration over heterogeneous menu types
 * for keyboard shortcut activation/deactivation and UI translation.
 */

#ifndef COCKATRICE_ABSTRACT_ZONE_MENU_H
#define COCKATRICE_ABSTRACT_ZONE_MENU_H

/**
 * @class AbstractZoneMenu
 * @brief Interface for zone menus that support keyboard shortcuts and translation.
 *
 * Provides a common contract for menus that need to:
 * - Activate/deactivate keyboard shortcuts based on game focus
 * - Update UI text when language changes
 *
 * Menus implementing this interface can be stored polymorphically
 * in PlayerMenu for batch operations.
 */
class AbstractZoneMenu
{
public:
    virtual ~AbstractZoneMenu() = default;

    /**
     * @brief Enable keyboard shortcuts for this menu's actions.
     *
     * Called when this player becomes the active/focused player.
     * Implementations should apply shortcut keys to their QActions.
     */
    virtual void setShortcutsActive() = 0;

    /**
     * @brief Disable keyboard shortcuts for this menu's actions.
     *
     * Called when focus moves away from this player.
     * Implementations should clear shortcut keys from their QActions.
     */
    virtual void setShortcutsInactive() = 0;

    /**
     * @brief Update all translatable text in the menu.
     *
     * Called when the application language changes.
     * Implementations should update menu titles, action text, and tooltips.
     */
    virtual void retranslateUi() = 0;
};

#endif // COCKATRICE_ABSTRACT_ZONE_MENU_H
