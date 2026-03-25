#include "move_menu.h"

#include "../card_menu_action_type.h"
#include "../player.h"
#include "../player_actions.h"

MoveMenu::MoveMenu(Player *player) : QMenu(tr("Move to"))
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

    connect(aMoveToTopLibrary, &QAction::triggered, player->getPlayerActions(), &PlayerActions::cardMenuAction);
    connect(aMoveToBottomLibrary, &QAction::triggered, player->getPlayerActions(), &PlayerActions::cardMenuAction);
    connect(aMoveToXfromTopOfLibrary, &QAction::triggered, player->getPlayerActions(),
            &PlayerActions::actMoveCardXCardsFromTop);
    connect(aMoveToTable, &QAction::triggered, player->getPlayerActions(), &PlayerActions::cardMenuAction);
    connect(aMoveToHand, &QAction::triggered, player->getPlayerActions(), &PlayerActions::cardMenuAction);
    connect(aMoveToGraveyard, &QAction::triggered, player->getPlayerActions(), &PlayerActions::cardMenuAction);
    connect(aMoveToExile, &QAction::triggered, player->getPlayerActions(), &PlayerActions::cardMenuAction);

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
}
