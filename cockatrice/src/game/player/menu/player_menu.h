#ifndef COCKATRICE_PLAYER_MENU_H
#define COCKATRICE_PLAYER_MENU_H

#include "../../../client/tearoff_menu.h"
#include "../player.h"
#include "custom_zone_menu.h"
#include "grave_menu.h"
#include "hand_menu.h"
#include "library_menu.h"
#include "rfg_menu.h"
#include "say_menu.h"
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
    void addPlayer(Player *playerToAdd);
    void removePlayer(Player *playerToRemove);
    void playerListActionTriggered();
    void refreshShortcuts();

public:
    PlayerMenu(Player *player);
    void retranslateUi();

    void addPlayerToList(QMenu *playerList, Player *playerToAdd);
    static void removePlayerFromList(QMenu *playerList, Player *player);

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
    void setShortcutIfItExists(QAction *action, ShortcutKey shortcut);
    void clearShortcutIfItExists(QAction *action);
    void setShortcutsInactive();

private:
    Player *player;
    TearOffMenu *playerMenu;
    QMenu *sbMenu, *countersMenu;
    HandMenu *handMenu;
    LibraryMenu *libraryMenu;
    GraveyardMenu *graveMenu;
    RfgMenu *rfgMenu;
    UtilityMenu *utilityMenu;
    SayMenu *sayMenu;
    CustomZoneMenu *customZonesMenu;
    QList<QMenu *> playerLists;
    QList<QAction *> allPlayersActions;
    QAction *aViewSideboard;

    bool shortcutsActive;

    void initSayMenu();
};

#endif // COCKATRICE_PLAYER_MENU_H
