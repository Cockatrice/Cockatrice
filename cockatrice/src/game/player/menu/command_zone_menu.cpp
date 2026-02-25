#include "command_zone_menu.h"

#include <QDebug>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CommandZoneMenuLog, "cockatrice.command_zone_menu")

#include "../../../client/settings/cache_settings.h"
#include "../../game_scene.h"
#include "../../zones/command_zone.h"
#include "../../zones/zone_names.h"
#include "../player.h"
#include "../player_actions.h"
#include "../player_graphics_item.h"

#include <libcockatrice/common/counter_ids.h>
#include <libcockatrice/protocol/pb/command_inc_counter.pb.h>

CommandZoneMenu::CommandZoneMenu(Player *_player, const QString &_zoneName, QMenu *playerMenu)
    : QMenu(playerMenu), player(_player), zoneName(_zoneName)
{
    // Initialize shortcut keys based on zone type
    if (zoneName == ZoneNames::COMMAND) {
        viewZoneShortcutKey = QStringLiteral("Player/aViewCommandZone");
        viewPartnerZoneShortcutKey = QStringLiteral("Player/aViewPartnerZone");
        incTaxShortcutKey = QStringLiteral("Player/aAddCommanderTax");
        decTaxShortcutKey = QStringLiteral("Player/aRemoveCommanderTax");
        incPartnerTaxShortcutKey = QStringLiteral("Player/aAddPartnerTax");
        decPartnerTaxShortcutKey = QStringLiteral("Player/aRemovePartnerTax");
        togglePartnerZoneShortcutKey = QStringLiteral("Player/aTogglePartnerZoneVisibility");
        toggleCompanionZoneShortcutKey = QStringLiteral("Player/aToggleCompanionZoneVisibility");
        toggleBackgroundZoneShortcutKey = QStringLiteral("Player/aToggleBackgroundZoneVisibility");
    } else if (zoneName == ZoneNames::PARTNER) {
        viewZoneShortcutKey = QStringLiteral("Player/aViewPartnerZone");
        incTaxShortcutKey = QStringLiteral("Player/aAddPartnerTax");
        decTaxShortcutKey = QStringLiteral("Player/aRemovePartnerTax");
        togglePartnerZoneShortcutKey = QStringLiteral("Player/aTogglePartnerZoneVisibility");
    } else if (zoneName == ZoneNames::COMPANION) {
        viewZoneShortcutKey = QStringLiteral("Player/aViewCompanionZone");
        toggleCompanionZoneShortcutKey = QStringLiteral("Player/aToggleCompanionZoneVisibility");
    } else if (zoneName == ZoneNames::BACKGROUND) {
        viewZoneShortcutKey = QStringLiteral("Player/aViewBackgroundZone");
        toggleBackgroundZoneShortcutKey = QStringLiteral("Player/aToggleBackgroundZoneVisibility");
    }

    aViewZone = new QAction(this);
    connect(aViewZone, &QAction::triggered, this,
            [this]() { player->getGameScene()->toggleZoneView(player, zoneName, -1); });

    if (zoneName == ZoneNames::COMMAND) {
        aViewPartnerZone = new QAction(this);
        connect(aViewPartnerZone, &QAction::triggered, this,
                [this]() { player->getGameScene()->toggleZoneView(player, ZoneNames::PARTNER, -1); });
    }

    if (player->getPlayerInfo()->getLocalOrJudge()) {
        addAction(aViewZone);
        if (aViewPartnerZone) {
            addAction(aViewPartnerZone);
        }
        addSeparator();

        if (zoneName == ZoneNames::COMMAND) {
            aIncreaseCommanderTax = new QAction(this);
            connect(aIncreaseCommanderTax, &QAction::triggered, this,
                    [this]() { modifyTaxCounter(CounterIds::CommanderTax, 1); });
            addAction(aIncreaseCommanderTax);

            aDecreaseCommanderTax = new QAction(this);
            connect(aDecreaseCommanderTax, &QAction::triggered, this,
                    [this]() { modifyTaxCounter(CounterIds::CommanderTax, -1); });
            addAction(aDecreaseCommanderTax);

            addSeparator();

            aIncreasePartnerTax = new QAction(this);
            connect(aIncreasePartnerTax, &QAction::triggered, this,
                    [this]() { modifyTaxCounter(CounterIds::PartnerTax, 1); });
            addAction(aIncreasePartnerTax);

            aDecreasePartnerTax = new QAction(this);
            connect(aDecreasePartnerTax, &QAction::triggered, this,
                    [this]() { modifyTaxCounter(CounterIds::PartnerTax, -1); });
            addAction(aDecreasePartnerTax);

            addSeparator();
        } else if (zoneName == ZoneNames::PARTNER) {
            aIncreaseCommanderTax = new QAction(this);
            connect(aIncreaseCommanderTax, &QAction::triggered, this,
                    [this]() { modifyTaxCounter(CounterIds::PartnerTax, 1); });
            addAction(aIncreaseCommanderTax);

            aDecreaseCommanderTax = new QAction(this);
            connect(aDecreaseCommanderTax, &QAction::triggered, this,
                    [this]() { modifyTaxCounter(CounterIds::PartnerTax, -1); });
            addAction(aDecreaseCommanderTax);

            addSeparator();
        }

        if (zoneName == ZoneNames::COMMAND) {
            aTogglePartnerZone = new QAction(this);
            connect(aTogglePartnerZone, &QAction::triggered, this, &CommandZoneMenu::actTogglePartnerZone);
            addAction(aTogglePartnerZone);

            aToggleCompanionZone = new QAction(this);
            connect(aToggleCompanionZone, &QAction::triggered, this, &CommandZoneMenu::actToggleCompanionZone);
            addAction(aToggleCompanionZone);

            aToggleBackgroundZone = new QAction(this);
            connect(aToggleBackgroundZone, &QAction::triggered, this, &CommandZoneMenu::actToggleBackgroundZone);
            addAction(aToggleBackgroundZone);

            connect(player, &Player::companionZoneSupportChanged, this,
                    &CommandZoneMenu::updateCompanionZoneToggleVisibility);
            connect(player, &Player::backgroundZoneSupportChanged, this,
                    &CommandZoneMenu::updateBackgroundZoneToggleVisibility);

            updateCompanionZoneToggleVisibility(player->hasServerCompanionZone());
            updateBackgroundZoneToggleVisibility(player->hasServerBackgroundZone());

            addSeparator();
        } else if (zoneName == ZoneNames::PARTNER) {
            aTogglePartnerZone = new QAction(this);
            connect(aTogglePartnerZone, &QAction::triggered, this, &CommandZoneMenu::actTogglePartnerZone);
            addAction(aTogglePartnerZone);

            addSeparator();
        } else if (zoneName == ZoneNames::COMPANION) {
            aToggleCompanionZone = new QAction(this);
            connect(aToggleCompanionZone, &QAction::triggered, this, &CommandZoneMenu::actToggleCompanionZone);
            addAction(aToggleCompanionZone);

            addSeparator();
        } else if (zoneName == ZoneNames::BACKGROUND) {
            aToggleBackgroundZone = new QAction(this);
            connect(aToggleBackgroundZone, &QAction::triggered, this, &CommandZoneMenu::actToggleBackgroundZone);
            addAction(aToggleBackgroundZone);

            addSeparator();
        }

        aToggleMinimized = new QAction(this);
        connect(aToggleMinimized, &QAction::triggered, this, &CommandZoneMenu::actToggleMinimized);
        addAction(aToggleMinimized);
    }

    retranslateUi();
}

void CommandZoneMenu::retranslateUi()
{
    if (zoneName == ZoneNames::COMMAND) {
        setTitle(tr("Co&mmander"));
        aViewZone->setText(tr("&View command zone"));
        if (aViewPartnerZone) {
            aViewPartnerZone->setText(tr("View &partner zone"));
        }
        if (aIncreaseCommanderTax) {
            aIncreaseCommanderTax->setText(tr("&Increase Commander Tax Counter (+1)"));
        }
        if (aDecreaseCommanderTax) {
            aDecreaseCommanderTax->setText(tr("&Decrease Commander Tax Counter (-1)"));
        }
        if (aIncreasePartnerTax) {
            aIncreasePartnerTax->setText(tr("Increase &Partner Tax Counter (+1)"));
        }
        if (aDecreasePartnerTax) {
            aDecreasePartnerTax->setText(tr("Decrease P&artner Tax Counter (-1)"));
        }
    } else if (zoneName == ZoneNames::PARTNER) {
        setTitle(tr("&Partner"));
        aViewZone->setText(tr("&View partner zone"));
        if (aIncreaseCommanderTax) {
            aIncreaseCommanderTax->setText(tr("&Increase Partner Tax Counter (+1)"));
        }
        if (aDecreaseCommanderTax) {
            aDecreaseCommanderTax->setText(tr("&Decrease Partner Tax Counter (-1)"));
        }
    } else if (zoneName == ZoneNames::COMPANION) {
        setTitle(tr("Co&mpanion"));
        aViewZone->setText(tr("&View companion zone"));
    } else if (zoneName == ZoneNames::BACKGROUND) {
        setTitle(tr("&Background"));
        aViewZone->setText(tr("&View background zone"));
    } else {
        qCWarning(CommandZoneMenuLog) << "CommandZoneMenu: Unknown zone name:" << zoneName;
        setTitle(zoneName);
        aViewZone->setText(tr("&View zone"));
    }

    if (aTogglePartnerZone) {
        aTogglePartnerZone->setText(tr("Toggle &Partner Zone"));
    }
    if (aToggleCompanionZone) {
        aToggleCompanionZone->setText(tr("Toggle C&ompanion Zone"));
    }
    if (aToggleBackgroundZone) {
        aToggleBackgroundZone->setText(tr("Toggle &Background Zone"));
    }
    if (aToggleMinimized) {
        aToggleMinimized->setText(tr("&Minimize"));
    }
}

void CommandZoneMenu::actTogglePartnerZone()
{
    CommandZone *partnerZone = player->getGraphicsItem()->getPartnerZoneGraphicsItem();
    if (partnerZone) {
        partnerZone->toggleExpanded();
    }
}

void CommandZoneMenu::actToggleCompanionZone()
{
    CommandZone *companionZone = player->getGraphicsItem()->getCompanionZoneGraphicsItem();
    if (companionZone) {
        companionZone->toggleExpanded();
    }
}

void CommandZoneMenu::actToggleBackgroundZone()
{
    CommandZone *backgroundZone = player->getGraphicsItem()->getBackgroundZoneGraphicsItem();
    if (backgroundZone) {
        backgroundZone->toggleExpanded();
    }
}

void CommandZoneMenu::actToggleMinimized()
{
    CommandZone *zone = nullptr;
    if (zoneName == ZoneNames::COMMAND) {
        zone = player->getGraphicsItem()->getCommandZoneGraphicsItem();
    } else if (zoneName == ZoneNames::PARTNER) {
        zone = player->getGraphicsItem()->getPartnerZoneGraphicsItem();
    } else if (zoneName == ZoneNames::COMPANION) {
        zone = player->getGraphicsItem()->getCompanionZoneGraphicsItem();
    } else if (zoneName == ZoneNames::BACKGROUND) {
        zone = player->getGraphicsItem()->getBackgroundZoneGraphicsItem();
    }
    if (zone) {
        zone->toggleMinimized();
    }
}

void CommandZoneMenu::modifyTaxCounter(int counterId, int delta)
{
    Command_IncCounter cmd;
    cmd.set_counter_id(counterId);
    cmd.set_delta(delta);
    player->getPlayerActions()->sendGameCommand(cmd);
}

void CommandZoneMenu::setShortcutsActive()
{
    ShortcutsSettings &shortcuts = SettingsCache::instance().shortcuts();

    aViewZone->setShortcuts(shortcuts.getShortcut(viewZoneShortcutKey));
    if (aViewPartnerZone)
        aViewPartnerZone->setShortcuts(shortcuts.getShortcut(viewPartnerZoneShortcutKey));

    if (aIncreaseCommanderTax)
        aIncreaseCommanderTax->setShortcuts(shortcuts.getShortcut(incTaxShortcutKey));
    if (aDecreaseCommanderTax)
        aDecreaseCommanderTax->setShortcuts(shortcuts.getShortcut(decTaxShortcutKey));
    if (aTogglePartnerZone)
        aTogglePartnerZone->setShortcuts(shortcuts.getShortcut(togglePartnerZoneShortcutKey));
    if (aToggleCompanionZone)
        aToggleCompanionZone->setShortcuts(shortcuts.getShortcut(toggleCompanionZoneShortcutKey));
    if (aToggleBackgroundZone)
        aToggleBackgroundZone->setShortcuts(shortcuts.getShortcut(toggleBackgroundZoneShortcutKey));
}

void CommandZoneMenu::setShortcutsInactive()
{
    aViewZone->setShortcut(QKeySequence());
    if (aViewPartnerZone)
        aViewPartnerZone->setShortcut(QKeySequence());

    if (aIncreaseCommanderTax)
        aIncreaseCommanderTax->setShortcut(QKeySequence());
    if (aDecreaseCommanderTax)
        aDecreaseCommanderTax->setShortcut(QKeySequence());
    if (aTogglePartnerZone)
        aTogglePartnerZone->setShortcut(QKeySequence());
    if (aToggleCompanionZone)
        aToggleCompanionZone->setShortcut(QKeySequence());
    if (aToggleBackgroundZone)
        aToggleBackgroundZone->setShortcut(QKeySequence());
}

void CommandZoneMenu::updateCompanionZoneToggleVisibility(bool hasCompanionZone)
{
    if (aToggleCompanionZone) {
        aToggleCompanionZone->setVisible(hasCompanionZone);
    }
}

void CommandZoneMenu::updateBackgroundZoneToggleVisibility(bool hasBackgroundZone)
{
    if (aToggleBackgroundZone) {
        aToggleBackgroundZone->setVisible(hasBackgroundZone);
    }
}
