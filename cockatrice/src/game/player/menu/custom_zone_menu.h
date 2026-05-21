/**
 * @file custom_zone_menu.h
 * @ingroup GameMenusZones
 */
//! \todo Document this file.

#ifndef COCKATRICE_CUSTOM_ZONE_MENU_H
#define COCKATRICE_CUSTOM_ZONE_MENU_H

#include "abstract_player_component.h"

#include <QMenu>

class PlayerGraphicsItem;
class CustomZoneMenu : public QMenu, public AbstractPlayerComponent
{
    Q_OBJECT
public:
    explicit CustomZoneMenu(PlayerGraphicsItem *player);
    void retranslateUi() override;
    void setShortcutsActive() override
    {
    }
    void setShortcutsInactive() override
    {
    }

private:
    PlayerGraphicsItem *player;
private slots:
    void clearCustomZonesMenu();
    void addViewCustomZoneActionToCustomZoneMenu(QString zoneName);
};

#endif // COCKATRICE_CUSTOM_ZONE_MENU_H
