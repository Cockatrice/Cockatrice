/**
 * @file player_menu.h
 * @ingroup GameMenusPlayers
 * @brief Orchestrates lifecycle management for all player-bound UI components.
 */

#ifndef COCKATRICE_PLAYER_MENU_H
#define COCKATRICE_PLAYER_MENU_H

#include "../../../interface/widgets/menus/tearoff_menu.h"
#include "../player_logic.h"
#include "command_zone_menu.h"
#include "custom_zone_menu.h"
#include "grave_menu.h"
#include "hand_menu.h"
#include "library_menu.h"
#include "rfg_menu.h"
#include "say_menu.h"
#include "sideboard_menu.h"
#include "utility_menu.h"

#include <QList>
#include <QMenu>
#include <QObject>

class CardItem;
class CardMenu;
class PlayerGraphicsItem;
class PlayerMenu : public QObject
{
    Q_OBJECT

signals:
    void cardMenuUpdated(CardMenu *cardMenu);
    void cardInfoRequested(const CardRef &cardRef);
    void shortcutsActivated();
    void shortcutsDeactivated();
    void retranslateRequested();

public slots:
    void setMenusForGraphicItems();
    QMenu *updateCardMenu(const CardItem *card);

private slots:
    void refreshShortcuts();

public:
    explicit PlayerMenu(PlayerGraphicsItem *player);
    /** @brief Retranslate all user-visible strings. Called on language change. */
    void retranslateUi();

    [[nodiscard]] QMenu *getPlayerMenu() const
    {
        return playerMenu;
    }

    [[nodiscard]] QMenu *getCountersMenu()
    {
        return countersMenu;
    }

    [[nodiscard]] LibraryMenu *getLibraryMenu() const
    {
        return libraryMenu;
    }

    [[nodiscard]] UtilityMenu *getUtilityMenu() const
    {
        return utilityMenu;
    }

    [[nodiscard]] bool getShortcutsActive() const
    {
        return shortcutsActive;
    }

    /** @brief Bind keyboard shortcuts. Called when this player gains focus. */
    void setShortcutsActive();
    /** @brief Unbind keyboard shortcuts. Called when this player loses focus. */
    void setShortcutsInactive();

private:
    PlayerGraphicsItem *player;
    TearOffMenu *playerMenu;
    QMenu *countersMenu;
    HandMenu *handMenu;
    LibraryMenu *libraryMenu;
    SideboardMenu *sideboardMenu;
    GraveyardMenu *graveMenu;
    RfgMenu *rfgMenu;
    UtilityMenu *utilityMenu;
    SayMenu *sayMenu;
    CommandZoneMenu *commandZoneMenu;
    CustomZoneMenu *customZonesMenu;

    /** @brief Drives AbstractPlayerComponent lifecycle delegation. Counters are iterated separately via
     * player->getCounters().
     */
    QList<AbstractPlayerComponent *> managedComponents;
    bool shortcutsActive = false;

    /** @brief Creates component, adds it as a submenu of playerMenu, and registers in managedComponents. */
    template <typename MenuT, typename... Args> MenuT *addManagedMenu(Args &&...args)
    {
        auto *menu = new MenuT(std::forward<Args>(args)...);
        playerMenu->addMenu(menu);
        managedComponents.append(menu);
        return menu;
    }

    /** @brief Creates component and registers in managedComponents, but does NOT add it as a submenu. */
    template <typename ComponentT, typename... Args> ComponentT *createManagedComponent(Args &&...args)
    {
        auto *component = new ComponentT(std::forward<Args>(args)...);
        managedComponents.append(component);
        return component;
    }
};

#endif // COCKATRICE_PLAYER_MENU_H
