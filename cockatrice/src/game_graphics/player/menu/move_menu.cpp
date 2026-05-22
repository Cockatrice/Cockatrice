#include "move_menu.h"

#include "../../game/player/player_actions.h"
#include "../../game/player/player_logic.h"
#include "../card_menu_action_type.h"
#include "../player_graphics_item.h"

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
    aMoveToCommandZone = new QAction(this);
    aMoveToCommandZone->setData(cmMoveToCommandZone);

    auto *actions = player->getLogic()->getPlayerActions();

    auto invoke = [player](CardMenuActionType type) {
        return [type, player]() {
            player->getLogic()->getPlayerActions()->cardMenuAction(player->getGameScene()->selectedCards(), type);
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
    addAction(aMoveToCommandZone);

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
    aMoveToCommandZone->setShortcuts(shortcuts.getShortcut("Player/aMoveToCommandZone"));
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
    aMoveToCommandZone->setText(tr("&Command Zone"));
}
