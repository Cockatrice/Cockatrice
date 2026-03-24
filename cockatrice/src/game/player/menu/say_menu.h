/**
 * @file say_menu.h
 * @ingroup GameMenusPlayers
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_SAY_MENU_H
#define COCKATRICE_SAY_MENU_H

#include "abstract_player_component.h"

#include <QMenu>

class Player;
class SayMenu : public QMenu, public AbstractPlayerComponent
{
    Q_OBJECT
public:
    explicit SayMenu(Player *player);

    void retranslateUi() override;
    void setShortcutsActive() override;
    void setShortcutsInactive() override;

private slots:
    void initSayMenu();

private:
    Player *player;
    bool shortcutsActive = false;
};

#endif // COCKATRICE_SAY_MENU_H
