#include "command_zone_menu.h"

#include "../../../client/settings/cache_settings.h"
#include "../../../client/settings/shortcuts_settings.h"
#include "../../board/abstract_counter.h"
#include "../../game/player/player_actions.h"
#include "../../game/player/player_logic.h"
#include "../../game_scene.h"
#include "../../zones/command_zone.h"
#include "../player_graphics_item.h"

#include <libcockatrice/utility/counter_ids.h>
#include <libcockatrice/utility/counter_limits.h>

CommandZoneMenu::CommandZoneMenu(PlayerGraphicsItem *_player, QMenu *playerMenu) : QMenu(playerMenu), player(_player)
{
    // Shortcuts only for first two tax counters (matching legacy behavior)
    incTax1ShortcutKey = QStringLiteral("Player/aAddCommanderTax");
    decTax1ShortcutKey = QStringLiteral("Player/aRemoveCommanderTax");
    incTax2ShortcutKey = QStringLiteral("Player/aAddPartnerTax");
    decTax2ShortcutKey = QStringLiteral("Player/aRemovePartnerTax");

    PlayerLogic *logic = player->getLogic();
    if (logic && logic->getPlayerInfo()->getLocalOrJudge()) {
        for (int i = 0; i < TaxCounterCount; ++i) {
            int counterId = CounterIds::taxCounterIdFromIndex(i);

            aIncreaseTax[i] = new QAction(this);
            connect(aIncreaseTax[i], &QAction::triggered, this, [this, counterId]() {
                if (auto *l = player->getLogic()) {
                    l->getPlayerActions()->actModifyTaxCounter(counterId, 1);
                }
            });
            addAction(aIncreaseTax[i]);

            aDecreaseTax[i] = new QAction(this);
            connect(aDecreaseTax[i], &QAction::triggered, this, [this, counterId]() {
                if (auto *l = player->getLogic()) {
                    l->getPlayerActions()->actModifyTaxCounter(counterId, -1);
                }
            });
            addAction(aDecreaseTax[i]);

            addSeparator();

            aToggleTax[i] = new QAction(this);
            connect(aToggleTax[i], &QAction::triggered, this, [this, counterId]() {
                if (auto *l = player->getLogic()) {
                    l->getPlayerActions()->actToggleTaxCounter(counterId);
                }
            });
            addAction(aToggleTax[i]);

            addSeparator();
        }

        aToggleMinimized = new QAction(this);
        connect(aToggleMinimized, &QAction::triggered, this, &CommandZoneMenu::actToggleMinimized);
        addAction(aToggleMinimized);

        connect(this, &QMenu::aboutToShow, this, &CommandZoneMenu::updateTaxCounterActionStates);
    }

    retranslateUi();
}

void CommandZoneMenu::retranslateUi()
{
    setTitle(tr("Co&mmander"));

    static const char *ordinals[] = {"1st", "2nd", "3rd", "4th", "5th"};

    for (int i = 0; i < TaxCounterCount; ++i) {
        if (aIncreaseTax[i]) {
            aIncreaseTax[i]->setText(tr("&Increase %1 Tax (+1)").arg(ordinals[i]));
        }
        if (aDecreaseTax[i]) {
            aDecreaseTax[i]->setText(tr("&Decrease %1 Tax (-1)").arg(ordinals[i]));
        }
        // Toggle action labels are derived dynamically in updateTaxCounterActionStates()
    }

    if (aToggleMinimized) {
        aToggleMinimized->setText(tr("&Minimize"));
    }
}

void CommandZoneMenu::actToggleMinimized()
{
    CommandZone *zone = player->getCommandZoneGraphicsItem();
    if (zone) {
        zone->toggleMinimized();
    }
}

void CommandZoneMenu::updateTaxCounterActionStates()
{
    static const char *ordinals[] = {"1st", "2nd", "3rd", "4th", "5th"};

    // Collect all tax counter states
    std::array<AbstractCounter *, TaxCounterCount> taxCounters{};
    for (int i = 0; i < TaxCounterCount; ++i) {
        taxCounters[i] = player->getTaxCounterIfActive(CounterIds::taxCounterIdFromIndex(i));
    }

    // Find highest active tax counter index
    int highestActive = -1;
    for (int i = TaxCounterCount - 1; i >= 0; --i) {
        if (taxCounters[i]) {
            highestActive = i;
            break;
        }
    }

    for (int i = 0; i < TaxCounterCount; ++i) {
        AbstractCounter *counter = taxCounters[i];

        if (aIncreaseTax[i]) {
            aIncreaseTax[i]->setVisible(counter && counter->getValue() < MAX_COUNTER_VALUE);
        }
        if (aDecreaseTax[i]) {
            aDecreaseTax[i]->setVisible(counter && counter->getValue() > 0);
        }
        if (aToggleTax[i]) {
            aToggleTax[i]->setText(counter ? tr("&Remove %1 Tax").arg(ordinals[i])
                                           : tr("&Add %1 Tax").arg(ordinals[i]));

            // Toggle visible if:
            // - Counter doesn't exist and previous counter is active (can add next in sequence)
            // - Counter exists with value 0 and is the highest active (can remove last in sequence)
            bool canAdd = !counter && (i == 0 || taxCounters[i - 1]);
            bool canRemove = counter && counter->getValue() == 0 && i == highestActive;
            aToggleTax[i]->setVisible(canAdd || canRemove);
        }
    }

    if (aToggleMinimized) {
        CommandZone *zone = player->getCommandZoneGraphicsItem();
        aToggleMinimized->setText(zone && zone->isMinimized() ? tr("&Restore") : tr("&Minimize"));
    }
}

void CommandZoneMenu::setShortcutsActive()
{
    ShortcutsSettings &shortcuts = SettingsCache::instance().shortcuts();

    // Only first two tax counters have shortcuts
    if (aIncreaseTax[0]) {
        aIncreaseTax[0]->setShortcuts(shortcuts.getShortcut(incTax1ShortcutKey));
    }
    if (aDecreaseTax[0]) {
        aDecreaseTax[0]->setShortcuts(shortcuts.getShortcut(decTax1ShortcutKey));
    }
    if (aIncreaseTax[1]) {
        aIncreaseTax[1]->setShortcuts(shortcuts.getShortcut(incTax2ShortcutKey));
    }
    if (aDecreaseTax[1]) {
        aDecreaseTax[1]->setShortcuts(shortcuts.getShortcut(decTax2ShortcutKey));
    }
}

void CommandZoneMenu::setShortcutsInactive()
{
    // Only first two tax counters have shortcuts
    if (aIncreaseTax[0]) {
        aIncreaseTax[0]->setShortcut(QKeySequence());
    }
    if (aDecreaseTax[0]) {
        aDecreaseTax[0]->setShortcut(QKeySequence());
    }
    if (aIncreaseTax[1]) {
        aIncreaseTax[1]->setShortcut(QKeySequence());
    }
    if (aDecreaseTax[1]) {
        aDecreaseTax[1]->setShortcut(QKeySequence());
    }
}
