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
    inc1ShortcutKey = QStringLiteral("Player/aIncrease1stCastCount");
    dec1ShortcutKey = QStringLiteral("Player/aDecrease1stCastCount");
    inc2ShortcutKey = QStringLiteral("Player/aIncrease2ndCastCount");
    dec2ShortcutKey = QStringLiteral("Player/aDecrease2ndCastCount");

    PlayerLogic *logic = player->getLogic();
    if (logic && logic->getPlayerInfo()->getLocalOrJudge()) {
        for (int i = 0; i < CastCountCount; ++i) {
            int index = i + 1; // 1-based index

            aIncrease[i] = new QAction(this);
            connect(aIncrease[i], &QAction::triggered, this, [this, index]() {
                if (auto *l = player->getLogic()) {
                    l->getPlayerActions()->actModifyCastCount(index, 1);
                }
            });
            addAction(aIncrease[i]);

            aDecrease[i] = new QAction(this);
            connect(aDecrease[i], &QAction::triggered, this, [this, index]() {
                if (auto *l = player->getLogic()) {
                    l->getPlayerActions()->actModifyCastCount(index, -1);
                }
            });
            addAction(aDecrease[i]);

            addSeparator();

            aToggle[i] = new QAction(this);
            connect(aToggle[i], &QAction::triggered, this, [this, index]() {
                if (auto *l = player->getLogic()) {
                    l->getPlayerActions()->actToggleCastCount(index);
                }
            });
            addAction(aToggle[i]);

            addSeparator();
        }

        aToggleMinimized = new QAction(this);
        connect(aToggleMinimized, &QAction::triggered, this, &CommandZoneMenu::actToggleMinimized);
        addAction(aToggleMinimized);

        connect(this, &QMenu::aboutToShow, this, &CommandZoneMenu::updateCastCountActionStates);
    }

    retranslateUi();
}

void CommandZoneMenu::retranslateUi()
{
    setTitle(tr("Co&mmander"));

    static const char *ordinals[] = {"1st", "2nd", "3rd", "4th", "5th"};

    for (int i = 0; i < CastCountCount; ++i) {
        if (aIncrease[i]) {
            aIncrease[i]->setText(tr("&Increase %1 Cast Count (+1)").arg(ordinals[i]));
        }
        if (aDecrease[i]) {
            aDecrease[i]->setText(tr("&Decrease %1 Cast Count (-1)").arg(ordinals[i]));
        }
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

void CommandZoneMenu::updateCastCountActionStates()
{
    static const char *ordinals[] = {"1st", "2nd", "3rd", "4th", "5th"};

    std::array<AbstractCounter *, CastCountCount> castCounts{};
    for (int i = 0; i < CastCountCount; ++i) {
        castCounts[i] = player->getCastCountWidget(i + 1);
    }

    int highestActive = -1;
    for (int i = CastCountCount - 1; i >= 0; --i) {
        if (castCounts[i]) {
            highestActive = i;
            break;
        }
    }

    for (int i = 0; i < CastCountCount; ++i) {
        AbstractCounter *counter = castCounts[i];

        if (aIncrease[i]) {
            aIncrease[i]->setVisible(counter && counter->getValue() < MAX_COUNTER_VALUE);
        }
        if (aDecrease[i]) {
            aDecrease[i]->setVisible(counter && counter->getValue() > 0);
        }
        if (aToggle[i]) {
            aToggle[i]->setText(counter ? tr("&Remove %1 Cast Count").arg(ordinals[i])
                                        : tr("&Add %1 Cast Count").arg(ordinals[i]));

            bool canAdd = !counter && (i == 0 || castCounts[i - 1]);
            bool canRemove = counter && counter->getValue() == 0 && i == highestActive;
            aToggle[i]->setVisible(canAdd || canRemove);
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

    if (aIncrease[0]) {
        aIncrease[0]->setShortcuts(shortcuts.getShortcut(inc1ShortcutKey));
    }
    if (aDecrease[0]) {
        aDecrease[0]->setShortcuts(shortcuts.getShortcut(dec1ShortcutKey));
    }
    if (aIncrease[1]) {
        aIncrease[1]->setShortcuts(shortcuts.getShortcut(inc2ShortcutKey));
    }
    if (aDecrease[1]) {
        aDecrease[1]->setShortcuts(shortcuts.getShortcut(dec2ShortcutKey));
    }
}

void CommandZoneMenu::setShortcutsInactive()
{
    if (aIncrease[0]) {
        aIncrease[0]->setShortcut(QKeySequence());
    }
    if (aDecrease[0]) {
        aDecrease[0]->setShortcut(QKeySequence());
    }
    if (aIncrease[1]) {
        aIncrease[1]->setShortcut(QKeySequence());
    }
    if (aDecrease[1]) {
        aDecrease[1]->setShortcut(QKeySequence());
    }
}
