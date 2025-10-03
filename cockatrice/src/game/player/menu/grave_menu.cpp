#include "grave_menu.h"

#include "../../abstract_game.h"
#include "../player.h"
#include "../player_actions.h"
#include "grave_menu.h"

#include <QAction>
#include <QMenu>

GraveyardMenu::GraveyardMenu(Player *_player, QWidget *parent) : TearOffMenu(parent), player(_player)
{
    createMoveActions();
    createViewActions();

    addAction(aViewGraveyard);

    if (player->getPlayerInfo()->local || player->getPlayerInfo()->judge) {
        mRevealRandomGraveyardCard = addMenu(QString());
        connect(mRevealRandomGraveyardCard, &QMenu::aboutToShow, this,
                &GraveyardMenu::populateRevealRandomMenuWithActivePlayers);

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

void GraveyardMenu::populateRevealRandomMenuWithActivePlayers()
{
    mRevealRandomGraveyardCard->clear();

    QAction *allPlayers = mRevealRandomGraveyardCard->addAction(tr("&All players"));
    allPlayers->setData(-1);
    connect(allPlayers, &QAction::triggered, this, &GraveyardMenu::onRevealRandomTriggered);

    mRevealRandomGraveyardCard->addSeparator();

    const auto &players = player->getGame()->getPlayerManager()->getPlayers().values();
    for (auto *other : players) {
        if (other == player)
            continue;
        QAction *a = mRevealRandomGraveyardCard->addAction(other->getPlayerInfo()->getName());
        a->setData(other->getPlayerInfo()->getId());
        connect(a, &QAction::triggered, this, &GraveyardMenu::onRevealRandomTriggered);
    }
}

void GraveyardMenu::onRevealRandomTriggered()
{
    if (auto *a = qobject_cast<QAction *>(sender())) {
        player->getPlayerActions()->actRevealRandomGraveyardCard(a->data().toInt());
    }
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
