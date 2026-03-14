/**
 * @file abstract_zone_menu.h
 * @ingroup GameMenusZones
 * @brief Polymorphic interface for zone menus managed by PlayerMenu.
 */

#ifndef COCKATRICE_ABSTRACT_ZONE_MENU_H
#define COCKATRICE_ABSTRACT_ZONE_MENU_H

/**
 * @brief Interface that all zone menus implement so PlayerMenu can manage them polymorphically.
 *
 * Not a QObject — avoids diamond inheritance with Qt's MOC. Each concrete zone menu
 * inherits QObject through its Qt base class (QMenu or TearOffMenu) and this interface
 * through regular multiple inheritance.
 */
class AbstractZoneMenu
{
public:
    virtual ~AbstractZoneMenu() = default;

    /// Bind keyboard shortcuts. Called when this player gains focus.
    virtual void setShortcutsActive() = 0;

    /// Unbind keyboard shortcuts. Called when this player loses focus.
    virtual void setShortcutsInactive() = 0;

    /// Retranslate all user-visible strings. Called on language change.
    virtual void retranslateUi() = 0;
};

#endif // COCKATRICE_ABSTRACT_ZONE_MENU_H
