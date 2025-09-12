#ifndef COCKATRICE_HAND_MENU_H
#define COCKATRICE_HAND_MENU_H

#include "../../../client/tearoff_menu.h"

#include <QAction>
#include <QMenu>

class Player;
class PlayerActions;

class HandMenu : public TearOffMenu
{
    Q_OBJECT
public:
    explicit HandMenu(Player *player, PlayerActions *actions, QWidget *parent = nullptr);
    void retranslateUi();
    void setShortcutsActive();
    void setShortcutsInactive();

    // expose useful actions/menus if PlayerMenu needs them
    QMenu *revealHandMenu() const
    {
        return mRevealHand;
    }
    QMenu *revealRandomHandCardMenu() const
    {
        return mRevealRandomHandCard;
    }
    QMenu *moveHandMenu() const
    {
        return mMoveHandMenu;
    }

private:
    Player *player;

    QAction *aViewHand = nullptr;
    QAction *aSortHand = nullptr;
    QAction *aMulligan = nullptr;

    QMenu *mRevealHand = nullptr;
    QMenu *mRevealRandomHandCard = nullptr;
    QMenu *mMoveHandMenu = nullptr;

    QAction *aMoveHandToTopLibrary = nullptr;
    QAction *aMoveHandToBottomLibrary = nullptr;
    QAction *aMoveHandToGrave = nullptr;
    QAction *aMoveHandToRfg = nullptr;
};

#endif // COCKATRICE_HAND_MENU_H
