/**
 * @file rfg_menu.h
 * @ingroup GameMenusZones
 */
//! \todo Document this file.

#ifndef COCKATRICE_RFG_MENU_H
#define COCKATRICE_RFG_MENU_H

#include "../../../interface/widgets/menus/tearoff_menu.h"
#include "abstract_player_component.h"

#include <QAction>
#include <QMenu>

class PlayerGraphicsItem;
class RfgMenu : public TearOffMenu, public AbstractPlayerComponent
{
    Q_OBJECT
public:
    explicit RfgMenu(PlayerGraphicsItem *player, QWidget *parent = nullptr);
    void createMoveActions();
    void createViewActions();
    void retranslateUi() override;
    void setShortcutsActive() override
    {
    }
    void setShortcutsInactive() override
    {
    }

    QMenu *moveRfgMenu = nullptr;

    QAction *aViewRfg = nullptr;
    QAction *aMoveRfgToTopLibrary = nullptr;
    QAction *aMoveRfgToBottomLibrary = nullptr;
    QAction *aMoveRfgToHand = nullptr;
    QAction *aMoveRfgToGrave = nullptr;

private:
    PlayerGraphicsItem *player;
};

#endif // COCKATRICE_RFG_MENU_H
