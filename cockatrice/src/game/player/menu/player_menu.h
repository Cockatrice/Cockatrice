/**
 * @file player_menu.h
 * @ingroup GameMenusPlayers
 * @brief Aggregates all zone-related menus for a player.
 */

#ifndef COCKATRICE_PLAYER_MENU_H
#define COCKATRICE_PLAYER_MENU_H

#include "../../../interface/widgets/menus/tearoff_menu.h"
#include "../player.h"
#include "abstract_zone_menu.h"
#include "command_zone_menu.h"
#include "custom_zone_menu.h"
#include "grave_menu.h"
#include "hand_menu.h"
#include "library_menu.h"
#include "rfg_menu.h"
#include "say_menu.h"
#include "sideboard_menu.h"
#include "utility_menu.h"

#include <QMenu>
#include <QObject>
#include <QVector>

class CardItem;

/**
 * @class PlayerMenu
 * @brief Central manager for a player's zone context menus.
 *
 * PlayerMenu creates and coordinates all zone-specific context menus for a player,
 * including hand, library, graveyard, exile, sideboard, and command zones.
 * It attaches menus to their corresponding graphics items and manages keyboard
 * shortcut activation/deactivation.
 *
 * For Commander format games, PlayerMenu creates two CommandZoneMenu instances:
 * one for the primary command zone and one for the partner zone. It also connects
 * the partner zone's expansion state to the command zone menu's toggle action.
 *
 * @see CommandZoneMenu
 * @see CardMenu
 */
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
    PlayerMenu(Player *player);
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

    void setShortcutsActive();
    void setShortcutsInactive();

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
    CommandZoneMenu *commandZoneMenu;
    CommandZoneMenu *partnerZoneMenu;
    CommandZoneMenu *companionZoneMenu;
    CommandZoneMenu *backgroundZoneMenu;

    QVector<AbstractZoneMenu *> allZoneMenus;

    bool shortcutsActive = false;

    void initSayMenu();
};

#endif // COCKATRICE_PLAYER_MENU_H
