#include "move_menu.h"

#include "../../../client/settings/cache_settings.h"
#include "../../../client/settings/shortcuts_settings.h"
#include "../../game_scene.h"
#include "../player_graphics_item.h"

#include <QAction>
#include <libcockatrice/game/player/card_menu_action_type.h>
#include <libcockatrice/game/player/player_actions.h>
#include <libcockatrice/game/player/player_logic.h>

MoveMenu::MoveMenu(PlayerGraphicsItem *player) : QMenu(tr("Move to"))
{
    aMoveToTopLibrary = new QAction(this);
    aMoveToTopLibrary->setData(cmMoveToTopLibrary);
    aMoveToBottomLibrary = new QAction(this);
    aMoveToBottomLibrary->setData(cmMoveToBottomLibrary);
    aMoveToXfromTopOfLibrary = new QAction(this);
    aMoveToTable = new QAction(this);
    aMoveToTable->setData(cmMoveToTable);
    aMoveToGraveyard = new QAction(this);
    aMoveToHand = new QAction(this);
    aMoveToHand->setData(cmMoveToHand);
    aMoveToGraveyard->setData(cmMoveToGraveyard);
    aMoveToExile = new QAction(this);
    aMoveToExile->setData(cmMoveToExile);
    aMoveToLimbo = new QAction(this);
    aMoveToLimbo->setData(cmMoveToLimbo);
    aMoveToRefuge = new QAction(this);
    aMoveToRefuge->setData(cmMoveToRefuge);
    aMoveToOblivion = new QAction(this);
    aMoveToOblivion->setData(cmMoveToOblivion);

    auto *actions = player->getLogic()->getPlayerActions();

    auto invoke = [player](CardMenuActionType type) {
        return [type, player]() {
            player->getLogic()->getPlayerActions()->cardMenuAction(player->getGameScene()->selectedCardStates(), type);
        };
    };

    connect(aMoveToTopLibrary, &QAction::triggered, actions, invoke(cmMoveToTopLibrary));
    connect(aMoveToBottomLibrary, &QAction::triggered, actions, invoke(cmMoveToBottomLibrary));
    connect(aMoveToXfromTopOfLibrary, &QAction::triggered, actions,
            &PlayerActions::actRequestMoveCardXCardsFromTopDialog);
    connect(aMoveToTable, &QAction::triggered, actions, invoke(cmMoveToTable));
    connect(aMoveToHand, &QAction::triggered, actions, invoke(cmMoveToHand));
    connect(aMoveToGraveyard, &QAction::triggered, actions, invoke(cmMoveToGraveyard));
    connect(aMoveToExile, &QAction::triggered, actions, invoke(cmMoveToExile));
    connect(aMoveToLimbo, &QAction::triggered, actions, invoke(cmMoveToLimbo));
    connect(aMoveToRefuge, &QAction::triggered, actions, invoke(cmMoveToRefuge));
    connect(aMoveToOblivion, &QAction::triggered, actions, invoke(cmMoveToOblivion));

    addAction(aMoveToTopLibrary);
    addAction(aMoveToXfromTopOfLibrary);
    addAction(aMoveToBottomLibrary);
    addSeparator();
    addAction(aMoveToTable);
    addSeparator();
    addAction(aMoveToHand);
    addSeparator();
    addAction(aMoveToGraveyard);
    addSeparator();
    addAction(aMoveToExile);
    addSeparator();
    addAction(aMoveToLimbo);
    addAction(aMoveToRefuge);
    addAction(aMoveToOblivion);

    setShortcutsActive();

    retranslateUi();
}

void MoveMenu::setShortcutsActive()
{
    ShortcutsSettings &shortcuts = SettingsCache::instance().shortcuts();

    aMoveToTopLibrary->setShortcuts(shortcuts.getShortcut("Player/aMoveToTopLibrary"));
    aMoveToBottomLibrary->setShortcuts(shortcuts.getShortcut("Player/aMoveToBottomLibrary"));
    aMoveToTable->setShortcuts(shortcuts.getShortcut("Player/aMoveToTable"));
    aMoveToHand->setShortcuts(shortcuts.getShortcut("Player/aMoveToHand"));
    aMoveToGraveyard->setShortcuts(shortcuts.getShortcut("Player/aMoveToGraveyard"));
    aMoveToExile->setShortcuts(shortcuts.getShortcut("Player/aMoveToExile"));
}

void MoveMenu::retranslateUi()
{
    aMoveToTopLibrary->setText(tr("&Top of library in random order"));
    aMoveToXfromTopOfLibrary->setText(tr("X cards from the top of library..."));
    aMoveToBottomLibrary->setText(tr("&Bottom of library in random order"));
    aMoveToTable->setText(tr("T&able"));
    aMoveToHand->setText(tr("&Hand"));
    aMoveToGraveyard->setText(tr("&Graveyard"));
    aMoveToExile->setText(tr("&Exile"));
    aMoveToLimbo->setText(tr("Limbo"));
    aMoveToRefuge->setText(tr("Refuge"));
    aMoveToOblivion->setText(tr("Oblivion"));
}
