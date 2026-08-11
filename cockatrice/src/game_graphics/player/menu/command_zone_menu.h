/**
 * @file command_zone_menu.h
 * @ingroup GameMenusZones
 * @brief Context menu for command zone right-click actions.
 */

#ifndef COCKATRICE_COMMAND_ZONE_MENU_H
#define COCKATRICE_COMMAND_ZONE_MENU_H

#include "abstract_player_component.h"

#include <QMenu>
#include <array>
#include <libcockatrice/utility/counter_ids.h>

class PlayerGraphicsItem;

/**
 * @class CommandZoneMenu
 * @brief Context menu for the command zone.
 *
 * Appears when right-clicking on the command zone. Provides actions for
 * adjusting tax counters (up to 5) and toggling minimized state.
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

private:
    static constexpr int TaxCounterCount = CounterIds::TaxCounterCount;

    std::array<QAction *, TaxCounterCount> aIncreaseTax{};
    std::array<QAction *, TaxCounterCount> aDecreaseTax{};
    std::array<QAction *, TaxCounterCount> aToggleTax{};
    QAction *aToggleMinimized = nullptr;

public slots:
    void updateTaxCounterActionStates();

private slots:
    void actToggleMinimized();

private:
    PlayerGraphicsItem *player;

    // Shortcuts only for first two tax counters
    QString incTax1ShortcutKey;
    QString decTax1ShortcutKey;
    QString incTax2ShortcutKey;
    QString decTax2ShortcutKey;
};

#endif // COCKATRICE_COMMAND_ZONE_MENU_H
