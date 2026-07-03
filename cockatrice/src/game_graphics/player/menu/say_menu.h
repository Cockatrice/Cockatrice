/**
 * @file say_menu.h
 * @ingroup GameMenusPlayers
 */
//! \todo Document this file.

#ifndef COCKATRICE_SAY_MENU_H
#define COCKATRICE_SAY_MENU_H

#include "abstract_player_component.h"

#include <QMenu>

class PlayerGraphicsItem;
class SayMenu : public QMenu, public AbstractPlayerComponent
{
    Q_OBJECT
public:
    explicit SayMenu(PlayerGraphicsItem *player);

    void retranslateUi() override;
    void setShortcutsActive() override;
    void setShortcutsInactive() override;

private slots:
    void initSayMenu();

private:
    PlayerGraphicsItem *player;
    bool shortcutsActive = false;
};

#endif // COCKATRICE_SAY_MENU_H
