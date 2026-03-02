/**
 * @file say_menu.h
 * @ingroup GameMenusPlayers
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_SAY_MENU_H
#define COCKATRICE_SAY_MENU_H

#include <QMenu>

class Player;
class SayMenu : public QMenu
{
    Q_OBJECT
public:
    explicit SayMenu(Player *player);
    void initSayMenu();

private:
    Player *player;
};

#endif // COCKATRICE_SAY_MENU_H
