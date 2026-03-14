/**
 * @file rfg_menu.h
 * @ingroup GameMenusZones
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_RFG_MENU_H
#define COCKATRICE_RFG_MENU_H

#include "../../../interface/widgets/menus/tearoff_menu.h"
#include "abstract_zone_menu.h"

#include <QAction>
#include <QMenu>

class Player;
class RfgMenu : public TearOffMenu, public AbstractZoneMenu
{
    Q_OBJECT
public:
    explicit RfgMenu(Player *player, QWidget *parent = nullptr);
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
    Player *player;
};

#endif // COCKATRICE_RFG_MENU_H
