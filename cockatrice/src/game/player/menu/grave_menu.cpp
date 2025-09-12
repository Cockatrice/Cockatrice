#include "grave_menu.h"

#include "../player.h"
#include "../player_actions.h"

GraveyardMenu::GraveyardMenu(Player *_player, QWidget *parent) : TearOffMenu(parent), player(_player)
{
    createMoveActions();
    createViewActions();

    addAction(aViewGraveyard);

    if (player->getPlayerInfo()->local || player->getPlayerInfo()->judge) {
        mRevealRandomGraveyardCard = addMenu(QString());
        QAction *newAction = mRevealRandomGraveyardCard->addAction(QString());
        newAction->setData(-1);
        connect(newAction, &QAction::triggered, player->getPlayerActions(),
                &PlayerActions::actRevealRandomGraveyardCard);
        emit newPlayerActionCreated(newAction);
        mRevealRandomGraveyardCard->addSeparator();

        addSeparator();
        moveGraveMenu = addTearOffMenu(QString());
        moveGraveMenu->addAction(aMoveGraveToTopLibrary);
        moveGraveMenu->addAction(aMoveGraveToBottomLibrary);
        moveGraveMenu->addSeparator();
        moveGraveMenu->addAction(aMoveGraveToHand);
        moveGraveMenu->addSeparator();
        moveGraveMenu->addAction(aMoveGraveToRfg);
    }

    retranslateUi();
}

void GraveyardMenu::createMoveActions()
{
    auto grave = player->getGraveZone();

    if (player->getPlayerInfo()->local || player->getPlayerInfo()->judge) {
        aMoveGraveToTopLibrary = new QAction(this);
        aMoveGraveToTopLibrary->setData(QList<QVariant>() << "deck" << 0);
        aMoveGraveToBottomLibrary = new QAction(this);
        aMoveGraveToBottomLibrary->setData(QList<QVariant>() << "deck" << -1);
        aMoveGraveToHand = new QAction(this);
        aMoveGraveToHand->setData(QList<QVariant>() << "hand" << 0);
        aMoveGraveToRfg = new QAction(this);
        aMoveGraveToRfg->setData(QList<QVariant>() << "rfg" << 0);

        connect(aMoveGraveToTopLibrary, &QAction::triggered, grave, &PileZoneLogic::moveAllToZone);
        connect(aMoveGraveToBottomLibrary, &QAction::triggered, grave, &PileZoneLogic::moveAllToZone);
        connect(aMoveGraveToHand, &QAction::triggered, grave, &PileZoneLogic::moveAllToZone);
        connect(aMoveGraveToRfg, &QAction::triggered, grave, &PileZoneLogic::moveAllToZone);
    }
}

void GraveyardMenu::createViewActions()
{
    PlayerActions *playerActions = player->getPlayerActions();

    aViewGraveyard = new QAction(this);
    connect(aViewGraveyard, &QAction::triggered, playerActions, &PlayerActions::actViewGraveyard);
}

void GraveyardMenu::retranslateUi()
{
    setTitle(tr("&Graveyard"));

    aViewGraveyard->setText(tr("&View graveyard"));

    if (player->getPlayerInfo()->getLocalOrJudge()) {
        moveGraveMenu->setTitle(tr("&Move graveyard to..."));
        aMoveGraveToTopLibrary->setText(tr("&Top of library"));
        aMoveGraveToBottomLibrary->setText(tr("&Bottom of library"));
        aMoveGraveToHand->setText(tr("&Hand"));
        aMoveGraveToRfg->setText(tr("&Exile"));
        mRevealRandomGraveyardCard->setTitle(tr("Reveal random card to..."));
    }
}

void GraveyardMenu::setShortcutsActive()
{
    ShortcutsSettings &shortcuts = SettingsCache::instance().shortcuts();

    aViewGraveyard->setShortcuts(shortcuts.getShortcut("Player/aViewGraveyard"));
}

void GraveyardMenu::setShortcutsInactive()
{
    aViewGraveyard->setShortcut(QKeySequence());
}