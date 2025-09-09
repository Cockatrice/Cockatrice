#ifndef COCKATRICE_PLAYER_MENU_H
#define COCKATRICE_PLAYER_MENU_H

#include "../../client/tearoff_menu.h"
#include "player.h"

#include <QMenu>
#include <QObject>

class PlayerMenu : public QObject
{
    Q_OBJECT

signals:
    void cardMenuUpdated(QMenu *cardMenu);

public slots:
    QMenu *createPtMenu() const;
    QMenu *createMoveMenu() const;
    void enableOpenInDeckEditorAction() const;
    void populatePredefinedTokensMenu();

private slots:
    void addPlayer(Player *playerToAdd);
    void removePlayer(Player *playerToRemove);
    void playerListActionTriggered();
    void refreshShortcuts();
    void clearCustomZonesMenu();
    void addViewCustomZoneActionToCustomZoneMenu(QString zoneName);
    void resetTopCardMenuActions();

public:
    PlayerMenu(Player *player);
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

    [[nodiscard]] bool isAlwaysRevealTopCardChecked()
    {
        return aAlwaysRevealTopCard->isChecked();
    }

    [[nodiscard]] bool isAlwaysLookAtTopCardChecked()
    {
        return aAlwaysLookAtTopCard->isChecked();
    }

    [[nodiscard]] QMenu *getPlayerMenu() const
    {
        return playerMenu;
    }

    [[nodiscard]] QMenu *getCountersMenu()
    {
        return countersMenu;
    }

    bool getShortcutsActive() const
    {
        return shortcutsActive;
    }

    void setShortcutsActive();
    void setShortcutsInactive();

private:
    Player *player;
    QMenu *sbMenu, *countersMenu, *sayMenu, *createPredefinedTokenMenu, *mRevealLibrary, *mLendLibrary, *mRevealTopCard,
        *mRevealHand, *mRevealRandomHandCard, *mRevealRandomGraveyardCard, *mCustomZones, *mCardCounters;
    TearOffMenu *moveGraveMenu, *moveRfgMenu, *graveMenu, *moveHandMenu, *handMenu, *libraryMenu, *topLibraryMenu,
        *bottomLibraryMenu, *rfgMenu, *playerMenu;
    QList<QMenu *> playerLists;
    QList<QMenu *> singlePlayerLists;
    QList<QAction *> allPlayersActions;
    QList<QPair<QString, int>> playersInfo;
    QAction *aMoveHandToTopLibrary, *aMoveHandToBottomLibrary, *aMoveHandToGrave, *aMoveHandToRfg,
        *aMoveGraveToTopLibrary, *aMoveGraveToBottomLibrary, *aMoveGraveToHand, *aMoveGraveToRfg, *aMoveRfgToTopLibrary,
        *aMoveRfgToBottomLibrary, *aMoveRfgToHand, *aMoveRfgToGrave, *aViewHand, *aViewLibrary, *aViewTopCards,
        *aViewBottomCards, *aAlwaysRevealTopCard, *aAlwaysLookAtTopCard, *aOpenDeckInDeckEditor,
        *aMoveTopCardToGraveyard, *aMoveTopCardToExile, *aMoveTopCardsToGraveyard, *aMoveTopCardsToExile,
        *aMoveTopCardsUntil, *aMoveTopCardToBottom, *aViewGraveyard, *aViewRfg, *aViewSideboard, *aDrawCard,
        *aDrawCards, *aUndoDraw, *aMulligan, *aShuffle, *aShuffleTopCards, *aShuffleBottomCards, *aMoveTopToPlay,
        *aMoveTopToPlayFaceDown, *aUntapAll, *aRollDie, *aCreateToken, *aCreateAnotherToken, *aMoveBottomToPlay,
        *aMoveBottomToPlayFaceDown, *aMoveBottomCardToTop, *aMoveBottomCardToGraveyard, *aMoveBottomCardToExile,
        *aMoveBottomCardsToGraveyard, *aMoveBottomCardsToExile, *aDrawBottomCard, *aDrawBottomCards;

    QList<QAction *> aAddCounter, aSetCounter, aRemoveCounter;
    QAction *aPlay, *aPlayFacedown, *aHide, *aTap, *aDoesntUntap, *aAttach, *aUnattach, *aDrawArrow, *aSetPT, *aResetPT,
        *aIncP, *aDecP, *aIncT, *aDecT, *aIncPT, *aDecPT, *aFlowP, *aFlowT, *aSetAnnotation, *aFlip, *aPeek, *aClone,
        *aMoveToTopLibrary, *aMoveToBottomLibrary, *aMoveToHand, *aMoveToGraveyard, *aMoveToExile,
        *aMoveToXfromTopOfLibrary, *aSelectAll, *aSelectRow, *aSelectColumn, *aSortHand, *aIncrementAllCardCounters;

    bool shortcutsActive;
    QStringList predefinedTokens;

    QMenu *createCardMenu(const CardItem *card);

    void addRelatedCardActions(const CardItem *card, QMenu *cardMenu);
    void addRelatedCardView(const CardItem *card, QMenu *cardMenu);

    void initSayMenu();
    void initContextualPlayersMenu(QMenu *menu);
};

#endif // COCKATRICE_PLAYER_MENU_H
