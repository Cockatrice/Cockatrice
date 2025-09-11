#ifndef COCKATRICE_PLAYER_MENU_H
#define COCKATRICE_PLAYER_MENU_H

#include "../../../client/tearoff_menu.h"
#include "../player.h"
#include "grave_menu.h"
#include "hand_menu.h"
#include "library_menu.h"
#include "rfg_menu.h"

#include <QMenu>
#include <QObject>

class PlayerMenu : public QObject
{
    Q_OBJECT

signals:
    void cardMenuUpdated(QMenu *cardMenu);

public slots:
    QMenu *createMoveMenu() const;
    void populatePredefinedTokensMenu();
    void setMenusForGraphicItems();

private slots:
    void addPlayer(Player *playerToAdd);
    void removePlayer(Player *playerToRemove);
    void playerListActionTriggered();
    void refreshShortcuts();
    void clearCustomZonesMenu();
    void addViewCustomZoneActionToCustomZoneMenu(QString zoneName);

public:
    PlayerMenu(Player *player);
    void createViewActions();
    void retranslateUi();

    void addPlayerToList(QMenu *playerList, Player *playerToAdd);
    static void removePlayerFromList(QMenu *playerList, Player *player);

    QMenu *updateCardMenu(const CardItem *card);

    [[nodiscard]] bool createAnotherTokenActionExists() const
    {
        return aCreateAnotherToken != nullptr;
    }

    void setAndEnableCreateAnotherTokenAction(QString text)
    {
        aCreateAnotherToken->setText(text);
        aCreateAnotherToken->setEnabled(true);
    }

    QStringList getPredefinedTokens() const
    {
        return predefinedTokens;
    }

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
    QMenu *sbMenu, *countersMenu, *sayMenu, *createPredefinedTokenMenu, *mCustomZones,
        *mCardCounters;
    HandMenu *handMenu;
    LibraryMenu *libraryMenu;
    GraveyardMenu *graveMenu;
    RfgMenu *rfgMenu;
    TearOffMenu *playerMenu;
    QList<QMenu *> playerLists;
    QList<QMenu *> singlePlayerLists;
    QList<QAction *> allPlayersActions;
    QList<QPair<QString, int>> playersInfo;
    QAction *aViewSideboard, *aUntapAll, *aRollDie, *aCreateToken, *aCreateAnotherToken;

    QList<QAction *> aAddCounter, aSetCounter, aRemoveCounter;
    QAction *aPlay, *aPlayFacedown, *aHide, *aTap, *aDoesntUntap, *aAttach, *aUnattach, *aDrawArrow, *aSetAnnotation, *aFlip, *aPeek, *aClone,
        *aMoveToTopLibrary, *aMoveToBottomLibrary, *aMoveToHand, *aMoveToGraveyard, *aMoveToExile,
        *aMoveToXfromTopOfLibrary, *aSelectAll, *aSelectRow, *aSelectColumn, *aIncrementAllCardCounters;

    bool shortcutsActive;
    QStringList predefinedTokens;

    QMenu *createCardMenu(const CardItem *card);

    void addRelatedCardActions(const CardItem *card, QMenu *cardMenu);
    void addRelatedCardView(const CardItem *card, QMenu *cardMenu);

    void initSayMenu();
    void initContextualPlayersMenu(QMenu *menu);
};

#endif // COCKATRICE_PLAYER_MENU_H
