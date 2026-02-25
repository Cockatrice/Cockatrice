#include "move_menu.h"

#include "../card_menu_action_type.h"
#include "../player.h"
#include "../player_actions.h"

MoveMenu::MoveMenu(Player *_player) : QMenu(tr("Move to")), player(_player)
{
    aMoveToTopLibrary = new QAction(this);
    aMoveToTopLibrary->setData(cmMoveToTopLibrary);
    aMoveToBottomLibrary = new QAction(this);
    aMoveToBottomLibrary->setData(cmMoveToBottomLibrary);
    aMoveToXfromTopOfLibrary = new QAction(this);
    aMoveToGraveyard = new QAction(this);
    aMoveToHand = new QAction(this);
    aMoveToHand->setData(cmMoveToHand);
    aMoveToGraveyard->setData(cmMoveToGraveyard);
    aMoveToExile = new QAction(this);
    aMoveToExile->setData(cmMoveToExile);
    aMoveToCommandZone = new QAction(this);
    aMoveToCommandZone->setData(cmMoveToCommandZone);
    aMoveToPartnerZone = new QAction(this);
    aMoveToPartnerZone->setData(cmMoveToPartnerZone);
    aMoveToCompanionZone = new QAction(this);
    aMoveToCompanionZone->setData(cmMoveToCompanionZone);
    aMoveToBackgroundZone = new QAction(this);
    aMoveToBackgroundZone->setData(cmMoveToBackgroundZone);
    aMoveToTable = new QAction(this);
    aMoveToTable->setData(cmMoveToTable);

    connect(aMoveToTopLibrary, &QAction::triggered, player->getPlayerActions(), &PlayerActions::cardMenuAction);
    connect(aMoveToBottomLibrary, &QAction::triggered, player->getPlayerActions(), &PlayerActions::cardMenuAction);
    connect(aMoveToXfromTopOfLibrary, &QAction::triggered, player->getPlayerActions(),
            &PlayerActions::actMoveCardXCardsFromTop);
    connect(aMoveToHand, &QAction::triggered, player->getPlayerActions(), &PlayerActions::cardMenuAction);
    connect(aMoveToGraveyard, &QAction::triggered, player->getPlayerActions(), &PlayerActions::cardMenuAction);
    connect(aMoveToExile, &QAction::triggered, player->getPlayerActions(), &PlayerActions::cardMenuAction);
    connect(aMoveToCommandZone, &QAction::triggered, player->getPlayerActions(), &PlayerActions::cardMenuAction);
    connect(aMoveToPartnerZone, &QAction::triggered, player->getPlayerActions(), &PlayerActions::cardMenuAction);
    connect(aMoveToCompanionZone, &QAction::triggered, player->getPlayerActions(), &PlayerActions::cardMenuAction);
    connect(aMoveToBackgroundZone, &QAction::triggered, player->getPlayerActions(), &PlayerActions::cardMenuAction);
    connect(aMoveToTable, &QAction::triggered, player->getPlayerActions(), &PlayerActions::cardMenuAction);

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
    commandSubmenu = new QMenu(this);
    commandSubmenu->addAction(aMoveToCommandZone);
    commandSubmenu->addAction(aMoveToPartnerZone);
    addMenu(commandSubmenu);
    addAction(aMoveToCompanionZone);
    addAction(aMoveToBackgroundZone);

    setShortcutsActive();

    retranslateUi();

    auto updateCommandZoneActionsVisibility = [this](bool has) {
        commandSubmenu->menuAction()->setVisible(has);
        aMoveToCommandZone->setVisible(has);
        aMoveToPartnerZone->setVisible(has);
    };
    auto updateCompanionZoneActionVisibility = [this](bool has) { aMoveToCompanionZone->setVisible(has); };
    auto updateBackgroundZoneActionVisibility = [this](bool has) { aMoveToBackgroundZone->setVisible(has); };

    connect(player, &Player::commandZoneSupportChanged, this, updateCommandZoneActionsVisibility);
    connect(player, &Player::companionZoneSupportChanged, this, updateCompanionZoneActionVisibility);
    connect(player, &Player::backgroundZoneSupportChanged, this, updateBackgroundZoneActionVisibility);

    updateCommandZoneActionsVisibility(player->hasServerCommandZone());
    updateCompanionZoneActionVisibility(player->hasServerCompanionZone());
    updateBackgroundZoneActionVisibility(player->hasServerBackgroundZone());
}

void MoveMenu::setShortcutsActive()
{
    ShortcutsSettings &shortcuts = SettingsCache::instance().shortcuts();

    aMoveToTopLibrary->setShortcuts(shortcuts.getShortcut("Player/aMoveToTopLibrary"));
    aMoveToBottomLibrary->setShortcuts(shortcuts.getShortcut("Player/aMoveToBottomLibrary"));
    aMoveToHand->setShortcuts(shortcuts.getShortcut("Player/aMoveToHand"));
    aMoveToGraveyard->setShortcuts(shortcuts.getShortcut("Player/aMoveToGraveyard"));
    aMoveToExile->setShortcuts(shortcuts.getShortcut("Player/aMoveToExile"));
    aMoveToCommandZone->setShortcuts(shortcuts.getShortcut("Player/aMoveToCommandZone"));
    aMoveToPartnerZone->setShortcuts(shortcuts.getShortcut("Player/aMoveToPartnerZone"));
    aMoveToCompanionZone->setShortcuts(shortcuts.getShortcut("Player/aMoveToCompanionZone"));
    aMoveToBackgroundZone->setShortcuts(shortcuts.getShortcut("Player/aMoveToBackgroundZone"));
    aMoveToTable->setShortcuts(shortcuts.getShortcut("Player/aMoveToTable"));
}

void MoveMenu::retranslateUi()
{
    aMoveToTopLibrary->setText(tr("&Top of library in random order"));
    aMoveToXfromTopOfLibrary->setText(tr("X cards from the top of library..."));
    aMoveToBottomLibrary->setText(tr("&Bottom of library in random order"));
    aMoveToHand->setText(tr("&Hand"));
    aMoveToGraveyard->setText(tr("&Graveyard"));
    aMoveToExile->setText(tr("&Exile"));
    commandSubmenu->setTitle(tr("Co&mmander"));
    aMoveToCommandZone->setText(tr("&Commander"));
    aMoveToPartnerZone->setText(tr("&Partner"));
    aMoveToCompanionZone->setText(tr("C&ompanion"));
    aMoveToBackgroundZone->setText(tr("&Background"));
    aMoveToTable->setText(tr("&Battlefield"));
}
