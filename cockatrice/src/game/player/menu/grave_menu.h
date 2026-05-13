/**
 * @file grave_menu.h
 * @ingroup GameMenusZones
 */
//! \todo Document this file.

#ifndef COCKATRICE_GRAVE_MENU_H
#define COCKATRICE_GRAVE_MENU_H

#include "../../../interface/widgets/menus/tearoff_menu.h"
#include "abstract_player_component.h"

#include <QAction>
#include <QMenu>

class PlayerLogic;
class GraveyardMenu : public TearOffMenu, public AbstractPlayerComponent
{
    Q_OBJECT
signals:
    void newPlayerActionCreated(QAction *action);

public:
    explicit GraveyardMenu(PlayerLogic *player, QWidget *parent = nullptr);
    void createMoveActions();
    void createViewActions();
    void populateRevealRandomMenuWithActivePlayers();
    void onRevealRandomTriggered();
    void retranslateUi() override;
    void setShortcutsActive() override;
    void setShortcutsInactive() override;

    QMenu *mRevealRandomGraveyardCard = nullptr;
    QMenu *moveGraveMenu = nullptr;

    QAction *aViewGraveyard = nullptr;
    QAction *aMoveGraveToTopLibrary = nullptr;
    QAction *aMoveGraveToBottomLibrary = nullptr;
    QAction *aMoveGraveToHand = nullptr;
    QAction *aMoveGraveToRfg = nullptr;

private:
    PlayerLogic *player;
};

#endif // COCKATRICE_GRAVE_MENU_H
