/**
 * @file library_menu.h
 * @ingroup GameMenusZones
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_LIBRARY_MENU_H
#define COCKATRICE_LIBRARY_MENU_H

#include "../../../interface/widgets/menus/tearoff_menu.h"

#include <QAction>
#include <QMenu>

class Player;
class PlayerActions;

class LibraryMenu : public TearOffMenu
{
    Q_OBJECT
public slots:
    void enableOpenInDeckEditorAction() const;
    void resetTopCardMenuActions();

public:
    LibraryMenu(Player *player, QWidget *parent = nullptr);
    void createDrawActions();
    void createShuffleActions();
    void createMoveActions();
    void createViewActions();
    void retranslateUi();
    void populateRevealLibraryMenuWithActivePlayers();
    void populateLendLibraryMenuWithActivePlayers();
    void populateRevealTopCardMenuWithActivePlayers();
    void onRevealLibraryTriggered();
    void onLendLibraryTriggered();
    void onRevealTopCardTriggered();
    void setShortcutsActive();
    void setShortcutsInactive();

    [[nodiscard]] bool isAlwaysRevealTopCardChecked() const
    {
        return aAlwaysRevealTopCard->isChecked();
    }

    [[nodiscard]] bool isAlwaysLookAtTopCardChecked() const
    {
        return aAlwaysLookAtTopCard->isChecked();
    }

    // expose useful actions/menus if PlayerMenu needs them
    QMenu *revealLibrary() const
    {
        return mRevealLibrary;
    }
    QMenu *lendLibraryMenu() const
    {
        return mLendLibrary;
    }
    QMenu *revealTopCardMenu() const
    {
        return mRevealTopCard;
    }

    QMenu *topLibraryMenu = nullptr;
    QMenu *bottomLibraryMenu = nullptr;

    // Expose submenus that PlayerMenu tracks in its lists
    QMenu *mRevealLibrary = nullptr;
    QMenu *mLendLibrary = nullptr;
    QMenu *mRevealTopCard = nullptr;

    QAction *aDrawCard = nullptr;
    QAction *aDrawCards = nullptr;
    QAction *aUndoDraw = nullptr;

    QAction *aShuffle = nullptr;
    QAction *aViewLibrary = nullptr;
    QAction *aViewTopCards = nullptr;
    QAction *aViewBottomCards = nullptr;
    QAction *aAlwaysRevealTopCard = nullptr;
    QAction *aAlwaysLookAtTopCard = nullptr;
    QAction *aOpenDeckInDeckEditor = nullptr;

    QAction *aMoveTopToPlay = nullptr;
    QAction *aMoveTopToPlayFaceDown = nullptr;
    QAction *aMoveTopCardToBottom = nullptr;
    QAction *aMoveTopCardToGraveyard = nullptr;
    QAction *aMoveTopCardToExile = nullptr;
    QAction *aMoveTopCardsToGraveyard = nullptr;
    QAction *aMoveTopCardsToExile = nullptr;
    QAction *aMoveTopCardsUntil = nullptr;
    QAction *aShuffleTopCards = nullptr;

    QAction *aDrawBottomCard = nullptr;
    QAction *aDrawBottomCards = nullptr;
    QAction *aMoveBottomToPlay = nullptr;
    QAction *aMoveBottomToPlayFaceDown = nullptr;
    QAction *aMoveBottomCardToTop = nullptr;
    QAction *aMoveBottomCardToGraveyard = nullptr;
    QAction *aMoveBottomCardToExile = nullptr;
    QAction *aMoveBottomCardsToGraveyard = nullptr;
    QAction *aMoveBottomCardsToExile = nullptr;
    QAction *aShuffleBottomCards = nullptr;

    int defaultNumberTopCards = 1;

private:
    Player *player;
};

#endif // COCKATRICE_LIBRARY_MENU_H
