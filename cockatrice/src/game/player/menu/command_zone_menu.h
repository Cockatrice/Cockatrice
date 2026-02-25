/**
 * @file command_zone_menu.h
 * @ingroup GameMenusZones
 * @brief Menu for command zone and related zone right-click actions.
 */

#ifndef COCKATRICE_COMMAND_ZONE_MENU_H
#define COCKATRICE_COMMAND_ZONE_MENU_H

#include "abstract_zone_menu.h"

#include <QLoggingCategory>
#include <QMenu>

Q_DECLARE_LOGGING_CATEGORY(CommandZoneMenuLog)

class Player;

/**
 * @class CommandZoneMenu
 * @brief Context menu for command zone interactions.
 *
 * This menu appears when right-clicking on command zones. It provides
 * zone-specific actions including viewing zone contents, toggling
 * zones, and managing tax counters.
 *
 * Menu structure varies by zone type:
 * - Command Zone: Toggle Partner Zone/Companion Zone/Background Zone, tax counter actions
 * - Partner Zone: Toggle Partner Zone (self only), tax counter actions
 * - Companion Zone: Toggle Companion Zone (self only), no tax actions
 * - Background Zone: Toggle Background Zone (self only), no tax actions
 *
 * Each zone's self-toggle uses the same action type as toggling from parent menus,
 * ensuring consistent labeling and keyboard shortcuts.
 *
 * @see PlayerMenu
 * @see CommandZone
 */
class CommandZoneMenu : public QMenu, public AbstractZoneMenu
{
    Q_OBJECT

public:
    explicit CommandZoneMenu(Player *player, const QString &zoneName, QMenu *playerMenu);
    void retranslateUi() override;
    void setShortcutsActive() override;
    void setShortcutsInactive() override;

    QAction *aViewZone = nullptr;
    QAction *aViewPartnerZone = nullptr;
    QAction *aTogglePartnerZone = nullptr;
    QAction *aToggleCompanionZone = nullptr;
    QAction *aToggleBackgroundZone = nullptr;
    QAction *aToggleMinimized = nullptr;
    QAction *aIncreaseCommanderTax = nullptr;
    QAction *aDecreaseCommanderTax = nullptr;
    QAction *aIncreasePartnerTax = nullptr;
    QAction *aDecreasePartnerTax = nullptr;

public slots:
    void updateCompanionZoneToggleVisibility(bool hasCompanionZone);
    void updateBackgroundZoneToggleVisibility(bool hasBackgroundZone);

private slots:
    void actTogglePartnerZone();
    void actToggleCompanionZone();
    void actToggleBackgroundZone();
    void actToggleMinimized();

private:
    void modifyTaxCounter(int counterId, int delta);
    Player *player;
    QString zoneName;

    QString viewZoneShortcutKey;
    QString viewPartnerZoneShortcutKey;
    QString incTaxShortcutKey;
    QString decTaxShortcutKey;
    QString incPartnerTaxShortcutKey;
    QString decPartnerTaxShortcutKey;
    QString togglePartnerZoneShortcutKey;
    QString toggleCompanionZoneShortcutKey;
    QString toggleBackgroundZoneShortcutKey;
};

#endif // COCKATRICE_COMMAND_ZONE_MENU_H
