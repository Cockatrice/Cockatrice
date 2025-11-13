/**
 * @file hand_menu.h
 * @ingroup GameMenusZones
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_HAND_MENU_H
#define COCKATRICE_HAND_MENU_H

#include "../../../interface/widgets/menus/tearoff_menu.h"

#include <QAction>
#include <QMenu>

class Player;
class PlayerActions;

class HandMenu : public TearOffMenu
{
    Q_OBJECT

public:
    HandMenu(Player *player, PlayerActions *actions, QWidget *parent = nullptr);

    QMenu *revealHandMenu() const
    {
        return mRevealHand;
    }
    QMenu *revealRandomHandCardMenu() const
    {
        return mRevealRandomHandCard;
    }

    void retranslateUi();
    void setShortcutsActive();
    void setShortcutsInactive();

private slots:
    void populateRevealHandMenuWithActivePlayers();
    void populateRevealRandomHandCardMenuWithActivePlayers();
    void onRevealHandTriggered();
    void onRevealRandomHandCardTriggered();

private:
    Player *player;

    QAction *aViewHand = nullptr;
    QAction *aMulligan = nullptr;

    QMenu *mSortHand = nullptr;
    QAction *aSortHandByName = nullptr;
    QAction *aSortHandByType = nullptr;
    QAction *aSortHandByManaValue = nullptr;

    QMenu *mRevealHand = nullptr;
    QAction *aRevealHandToAll = nullptr;

    QMenu *mRevealRandomHandCard = nullptr;
    QAction *aRevealRandomHandCardToAll = nullptr;

    QMenu *mMoveHandMenu = nullptr;
    QAction *aMoveHandToTopLibrary = nullptr;
    QAction *aMoveHandToBottomLibrary = nullptr;
    QAction *aMoveHandToGrave = nullptr;
    QAction *aMoveHandToRfg = nullptr;
};

#endif // COCKATRICE_HAND_MENU_H
