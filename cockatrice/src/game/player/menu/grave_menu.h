/**
 * @file grave_menu.h
 * @ingroup GameMenusZones
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_GRAVE_MENU_H
#define COCKATRICE_GRAVE_MENU_H

#include "../../../interface/widgets/menus/tearoff_menu.h"

#include <QAction>
#include <QMenu>

class Player;
class GraveyardMenu : public TearOffMenu
{
    Q_OBJECT
signals:
    void newPlayerActionCreated(QAction *action);

public:
    explicit GraveyardMenu(Player *player, QWidget *parent = nullptr);
    void createMoveActions();
    void createViewActions();
    void populateRevealRandomMenuWithActivePlayers();
    void onRevealRandomTriggered();
    void retranslateUi();
    void setShortcutsActive();
    void setShortcutsInactive();

    QMenu *mRevealRandomGraveyardCard = nullptr;
    QMenu *moveGraveMenu = nullptr;

    QAction *aViewGraveyard = nullptr;
    QAction *aMoveGraveToTopLibrary = nullptr;
    QAction *aMoveGraveToBottomLibrary = nullptr;
    QAction *aMoveGraveToHand = nullptr;
    QAction *aMoveGraveToRfg = nullptr;

private:
    Player *player;
};

#endif // COCKATRICE_GRAVE_MENU_H
