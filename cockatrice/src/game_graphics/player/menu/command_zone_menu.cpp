#include "command_zone_menu.h"

#include "../../../client/settings/cache_settings.h"
#include "../../board/abstract_counter.h"
#include "../../game/player/player_actions.h"
#include "../../game/player/player_logic.h"
#include "../../game_scene.h"
#include "../../zones/command_zone.h"
#include "../player_graphics_item.h"

#include <libcockatrice/utility/counter_ids.h>
#include <libcockatrice/utility/counter_limits.h>
#include <libcockatrice/utility/zone_names.h>

CommandZoneMenu::CommandZoneMenu(PlayerGraphicsItem *_player, QMenu *playerMenu) : QMenu(playerMenu), player(_player)
{
    viewZoneShortcutKey = QStringLiteral("Player/aViewCommandZone");
    incTaxShortcutKey = QStringLiteral("Player/aAddCommanderTax");
    decTaxShortcutKey = QStringLiteral("Player/aRemoveCommanderTax");
    incPartnerTaxShortcutKey = QStringLiteral("Player/aAddPartnerTax");
    decPartnerTaxShortcutKey = QStringLiteral("Player/aRemovePartnerTax");

    aViewZone = new QAction(this);
    connect(aViewZone, &QAction::triggered, this, [this]() {
        if (PlayerLogic *logic = player->getLogic()) {
            emit logic->requestZoneViewToggle(logic, ZoneNames::COMMAND, -1, false);
        }
    });

    PlayerLogic *logic = player->getLogic();
    if (logic && logic->getPlayerInfo()->getLocalOrJudge()) {
        addAction(aViewZone);
        addSeparator();

        aIncreaseCommanderTax = new QAction(this);
        connect(aIncreaseCommanderTax, &QAction::triggered, this, [this]() {
            if (auto *logic = player->getLogic()) {
                logic->getPlayerActions()->actModifyTaxCounter(CounterIds::CommanderTax, 1);
            }
        });
        addAction(aIncreaseCommanderTax);

        aDecreaseCommanderTax = new QAction(this);
        connect(aDecreaseCommanderTax, &QAction::triggered, this, [this]() {
            if (auto *logic = player->getLogic()) {
                logic->getPlayerActions()->actModifyTaxCounter(CounterIds::CommanderTax, -1);
            }
        });
        addAction(aDecreaseCommanderTax);

        addSeparator();

        aIncreasePartnerTax = new QAction(this);
        connect(aIncreasePartnerTax, &QAction::triggered, this, [this]() {
            if (auto *logic = player->getLogic()) {
                logic->getPlayerActions()->actModifyTaxCounter(CounterIds::PartnerTax, 1);
            }
        });
        addAction(aIncreasePartnerTax);

        aDecreasePartnerTax = new QAction(this);
        connect(aDecreasePartnerTax, &QAction::triggered, this, [this]() {
            if (auto *logic = player->getLogic()) {
                logic->getPlayerActions()->actModifyTaxCounter(CounterIds::PartnerTax, -1);
            }
        });
        addAction(aDecreasePartnerTax);

        addSeparator();

        aToggleCommanderTaxCounter = new QAction(this);
        connect(aToggleCommanderTaxCounter, &QAction::triggered, this, [this]() {
            if (auto *logic = player->getLogic()) {
                logic->getPlayerActions()->actToggleTaxCounter(CounterIds::CommanderTax);
            }
        });
        addAction(aToggleCommanderTaxCounter);

        aTogglePartnerTaxCounter = new QAction(this);
        connect(aTogglePartnerTaxCounter, &QAction::triggered, this, [this]() {
            if (auto *logic = player->getLogic()) {
                logic->getPlayerActions()->actToggleTaxCounter(CounterIds::PartnerTax);
            }
        });
        addAction(aTogglePartnerTaxCounter);

        addSeparator();

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
    if (aViewZone) {
        aViewZone->setText(tr("&View command zone"));
    }
    if (aIncreaseCommanderTax) {
        aIncreaseCommanderTax->setText(tr("&Increase Commander Tax (+1)"));
    }
    if (aDecreaseCommanderTax) {
        aDecreaseCommanderTax->setText(tr("&Decrease Commander Tax (-1)"));
    }
    if (aIncreasePartnerTax) {
        aIncreasePartnerTax->setText(tr("Increase &Partner Tax (+1)"));
    }
    if (aDecreasePartnerTax) {
        aDecreasePartnerTax->setText(tr("Decrease P&artner Tax (-1)"));
    }
    // Toggle action labels are derived dynamically in updateTaxCounterActionStates()
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
    AbstractCounter *cmdTax = player->getTaxCounterIfActive(CounterIds::CommanderTax);
    AbstractCounter *partnerTax = player->getTaxCounterIfActive(CounterIds::PartnerTax);

    if (aIncreaseCommanderTax) {
        aIncreaseCommanderTax->setVisible(cmdTax && cmdTax->getValue() < MAX_COUNTER_VALUE);
    }
    if (aDecreaseCommanderTax) {
        aDecreaseCommanderTax->setVisible(cmdTax && cmdTax->getValue() > 0);
    }
    if (aToggleCommanderTaxCounter) {
        aToggleCommanderTaxCounter->setText(cmdTax ? tr("&Remove Commander Tax") : tr("&Add Commander Tax"));
        aToggleCommanderTaxCounter->setVisible(!cmdTax || cmdTax->getValue() == 0);
    }

    if (aIncreasePartnerTax) {
        aIncreasePartnerTax->setVisible(partnerTax && partnerTax->getValue() < MAX_COUNTER_VALUE);
    }
    if (aDecreasePartnerTax) {
        aDecreasePartnerTax->setVisible(partnerTax && partnerTax->getValue() > 0);
    }
    if (aTogglePartnerTaxCounter) {
        aTogglePartnerTaxCounter->setText(partnerTax ? tr("R&emove Partner Tax") : tr("&Add Partner Tax"));
        aTogglePartnerTaxCounter->setVisible(!partnerTax || partnerTax->getValue() == 0);
    }

    if (aToggleMinimized) {
        CommandZone *zone = player->getCommandZoneGraphicsItem();
        aToggleMinimized->setText(zone && zone->isMinimized() ? tr("&Restore") : tr("&Minimize"));
    }
}

void CommandZoneMenu::setShortcutsActive()
{
    ShortcutsSettings &shortcuts = SettingsCache::instance().shortcuts();

    if (aViewZone) {
        aViewZone->setShortcuts(shortcuts.getShortcut(viewZoneShortcutKey));
    }
    if (aIncreaseCommanderTax) {
        aIncreaseCommanderTax->setShortcuts(shortcuts.getShortcut(incTaxShortcutKey));
    }
    if (aDecreaseCommanderTax) {
        aDecreaseCommanderTax->setShortcuts(shortcuts.getShortcut(decTaxShortcutKey));
    }
    if (aIncreasePartnerTax) {
        aIncreasePartnerTax->setShortcuts(shortcuts.getShortcut(incPartnerTaxShortcutKey));
    }
    if (aDecreasePartnerTax) {
        aDecreasePartnerTax->setShortcuts(shortcuts.getShortcut(decPartnerTaxShortcutKey));
    }
}

void CommandZoneMenu::setShortcutsInactive()
{
    if (aViewZone) {
        aViewZone->setShortcut(QKeySequence());
    }
    if (aIncreaseCommanderTax) {
        aIncreaseCommanderTax->setShortcut(QKeySequence());
    }
    if (aDecreaseCommanderTax) {
        aDecreaseCommanderTax->setShortcut(QKeySequence());
    }
    if (aIncreasePartnerTax) {
        aIncreasePartnerTax->setShortcut(QKeySequence());
    }
    if (aDecreasePartnerTax) {
        aDecreasePartnerTax->setShortcut(QKeySequence());
    }
}
