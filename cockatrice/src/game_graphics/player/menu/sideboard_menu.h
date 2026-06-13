/**
 * @file sideboard_menu.h
 * @ingroup GameMenusZones
 */
//! \todo Document this file.

#ifndef COCKATRICE_SIDEBOARD_MENU_H
#define COCKATRICE_SIDEBOARD_MENU_H

#include "abstract_player_component.h"

#include <QMenu>

class PlayerGraphicsItem;
class SideboardMenu : public QMenu, public AbstractPlayerComponent
{
    Q_OBJECT

public:
    explicit SideboardMenu(PlayerGraphicsItem *player, QMenu *playerMenu);
    void retranslateUi() override;
    void setShortcutsActive() override;
    void setShortcutsInactive() override;

private:
    PlayerGraphicsItem *player;

    QAction *aViewSideboard;
};

#endif // COCKATRICE_SIDEBOARD_MENU_H
