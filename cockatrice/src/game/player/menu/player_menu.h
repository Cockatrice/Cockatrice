/**
 * @file player_menu.h
 * @ingroup GameMenusPlayers
 * @brief TODO: Document this.
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

    bool getShortcutsActive() const
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

    bool shortcutsActive;

    void initSayMenu();
};

#endif // COCKATRICE_PLAYER_MENU_H
