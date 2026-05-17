/**
 * @file player_menu.h
 * @ingroup GameMenusPlayers
 * @brief Orchestrates lifecycle management for all player-bound UI components.
 */

#ifndef COCKATRICE_PLAYER_MENU_H
#define COCKATRICE_PLAYER_MENU_H

#include "../../../interface/widgets/menus/tearoff_menu.h"
#include "../player.h"
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
class PlayerMenu : public QObject
{
    Q_OBJECT

signals:
    void cardMenuUpdated(QMenu *cardMenu);

public slots:
    void setMenusForGraphicItems();

private slots:
    void refreshShortcuts();

public:
    explicit PlayerMenu(Player *player);
    /// Lifecycle methods: delegate to all managedComponents, plus counters separately via player->getCounters().
    void retranslateUi();

    QMenu *updateCardMenu(const CardItem *card);

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

    /// Delegates to all managedComponents, plus counters separately.
    /// Stop full activation until game has started; only roll-die shortcut is active in lobby.
    void setShortcutsActive();
    /// Delegates to all managedComponents, plus counters separately.
    void setShortcutsInactive();

private slots:
    /// Re-evaluate shortcut state when game started/stopped state changes.
    void onGameStartedChanged(bool started);

private:
    Player *player;
    TearOffMenu *playerMenu;
    QMenu *countersMenu;
    HandMenu *handMenu;
    LibraryMenu *libraryMenu;
    SideboardMenu *sideboardMenu;
    GraveyardMenu *graveMenu;
    RfgMenu *rfgMenu;
    UtilityMenu *utilityMenu;
    SayMenu *sayMenu;
    CustomZoneMenu *customZonesMenu;

    /// Drives AbstractPlayerComponent lifecycle delegation. Counters are iterated separately via player->getCounters().
    QList<AbstractPlayerComponent *> managedComponents;
    bool shortcutsActive = false;

    /// Creates component, adds it as a submenu of playerMenu, and registers in managedComponents.
    template <typename MenuT, typename... Args> MenuT *addManagedMenu(Args &&...args)
    {
        auto *menu = new MenuT(std::forward<Args>(args)...);
        playerMenu->addMenu(menu);
        managedComponents.append(menu);
        return menu;
    }

    /// Creates component and registers in managedComponents, but does NOT add it as a submenu.
    template <typename ComponentT, typename... Args> ComponentT *createManagedComponent(Args &&...args)
    {
        auto *component = new ComponentT(std::forward<Args>(args)...);
        managedComponents.append(component);
        return component;
    }
};

#endif // COCKATRICE_PLAYER_MENU_H
