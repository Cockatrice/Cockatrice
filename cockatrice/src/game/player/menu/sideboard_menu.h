/**
 * @file sideboard_menu.h
 * @ingroup GameMenusZones
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_SIDEBOARD_MENU_H
#define COCKATRICE_SIDEBOARD_MENU_H

#include <QMenu>

class Player;
class SideboardMenu : public QMenu
{
    Q_OBJECT

public:
    explicit SideboardMenu(Player *player, QMenu *playerMenu);
    void retranslateUi();
    void setShortcutsActive();
    void setShortcutsInactive();

private:
    Player *player;

    QAction *aViewSideboard;
};

#endif // COCKATRICE_SIDEBOARD_MENU_H
