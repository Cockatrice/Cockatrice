/**
 * @file sideboard_menu.h
 * @ingroup GameMenusZones
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_SIDEBOARD_MENU_H
#define COCKATRICE_SIDEBOARD_MENU_H

#include "abstract_zone_menu.h"

#include <QMenu>

class Player;
class SideboardMenu : public QMenu, public AbstractZoneMenu
{
    Q_OBJECT

public:
    explicit SideboardMenu(Player *player, QMenu *playerMenu);
    void retranslateUi() override;
    void setShortcutsActive() override;
    void setShortcutsInactive() override;

private:
    Player *player;

    QAction *aViewSideboard;
};

#endif // COCKATRICE_SIDEBOARD_MENU_H
