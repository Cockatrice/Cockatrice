/**
 * @file command_zone_menu.h
 * @ingroup GameMenusZones
 * @brief Context menu for command zone right-click actions.
 */

#ifndef COCKATRICE_COMMAND_ZONE_MENU_H
#define COCKATRICE_COMMAND_ZONE_MENU_H

#include "abstract_player_component.h"

#include <QMenu>

class PlayerGraphicsItem;

/**
 * @class CommandZoneMenu
 * @brief Context menu for the command zone.
 *
 * Appears when right-clicking on the command zone. Provides actions for
 * viewing zone contents, adjusting the commander tax counter, and
 * toggling minimized state.
 *
 * @see PlayerMenu
 * @see CommandZone
 */
class CommandZoneMenu : public QMenu, public AbstractPlayerComponent
{
    Q_OBJECT

public:
    explicit CommandZoneMenu(PlayerGraphicsItem *player, QMenu *playerMenu);
    void retranslateUi() override;
    void setShortcutsActive() override;
    void setShortcutsInactive() override;

    QAction *aViewZone = nullptr; ///< Opens a zone viewer for the command zone

private:
    QAction *aIncreaseCommanderTax = nullptr;      ///< Increments the primary commander tax counter
    QAction *aDecreaseCommanderTax = nullptr;      ///< Decrements the primary commander tax counter
    QAction *aToggleCommanderTaxCounter = nullptr; ///< Toggles primary commander tax counter visibility
    QAction *aIncreasePartnerTax = nullptr;        ///< Increments the partner commander tax counter
    QAction *aDecreasePartnerTax = nullptr;        ///< Decrements the partner commander tax counter
    QAction *aTogglePartnerTaxCounter = nullptr;   ///< Toggles partner commander tax counter visibility
    QAction *aToggleMinimized = nullptr;           ///< Toggles command zone minimized state

private slots:
    void actToggleMinimized();

private:
    void updateTaxCounterActionStates();
    PlayerGraphicsItem *player;

    QString viewZoneShortcutKey;
    QString incTaxShortcutKey;
    QString decTaxShortcutKey;
    QString incPartnerTaxShortcutKey;
    QString decPartnerTaxShortcutKey;
};

#endif // COCKATRICE_COMMAND_ZONE_MENU_H
