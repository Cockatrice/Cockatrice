/**
 * @file custom_zone_menu.h
 * @ingroup GameMenusZones
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_CUSTOM_ZONE_MENU_H
#define COCKATRICE_CUSTOM_ZONE_MENU_H

#include <QMenu>

class Player;
class CustomZoneMenu : public QMenu
{
    Q_OBJECT
public:
    explicit CustomZoneMenu(Player *player);
    void retranslateUi();

private:
    Player *player;
private slots:
    void clearCustomZonesMenu();
    void addViewCustomZoneActionToCustomZoneMenu(QString zoneName);
};

#endif // COCKATRICE_CUSTOM_ZONE_MENU_H
