#include "rfg_menu.h"

#include "../player.h"
#include "../player_actions.h"

RfgMenu::RfgMenu(Player *_player, QWidget *parent) : TearOffMenu(parent), player(_player)
{
    createMoveActions();
    createViewActions();

    addAction(aViewRfg);

    if (player->getPlayerInfo()->getLocalOrJudge()) {
        addSeparator();
        moveRfgMenu = addTearOffMenu(QString());
        moveRfgMenu->addAction(aMoveRfgToTopLibrary);
        moveRfgMenu->addAction(aMoveRfgToBottomLibrary);
        moveRfgMenu->addSeparator();
        moveRfgMenu->addAction(aMoveRfgToHand);
        moveRfgMenu->addSeparator();
        moveRfgMenu->addAction(aMoveRfgToGrave);
    }

    retranslateUi();
}

void RfgMenu::createMoveActions()
{
    if (player->getPlayerInfo()->getLocalOrJudge()) {
        auto rfg = player->getRfgZone();

        aMoveRfgToTopLibrary = new QAction(this);
        aMoveRfgToTopLibrary->setData(QList<QVariant>() << "deck" << 0);
        aMoveRfgToBottomLibrary = new QAction(this);
        aMoveRfgToBottomLibrary->setData(QList<QVariant>() << "deck" << -1);
        aMoveRfgToHand = new QAction(this);
        aMoveRfgToHand->setData(QList<QVariant>() << "hand" << 0);
        aMoveRfgToGrave = new QAction(this);
        aMoveRfgToGrave->setData(QList<QVariant>() << "grave" << 0);

        connect(aMoveRfgToTopLibrary, &QAction::triggered, rfg, &PileZoneLogic::moveAllToZone);
        connect(aMoveRfgToBottomLibrary, &QAction::triggered, rfg, &PileZoneLogic::moveAllToZone);
        connect(aMoveRfgToHand, &QAction::triggered, rfg, &PileZoneLogic::moveAllToZone);
        connect(aMoveRfgToGrave, &QAction::triggered, rfg, &PileZoneLogic::moveAllToZone);
    }
}

void RfgMenu::createViewActions()
{
    PlayerActions *playerActions = player->getPlayerActions();

    aViewRfg = new QAction(this);
    connect(aViewRfg, &QAction::triggered, playerActions, &PlayerActions::actViewRfg);
}

void RfgMenu::retranslateUi()
{
    setTitle(tr("&Exile"));

    aViewRfg->setText(tr("&View exile"));

    if (player->getPlayerInfo()->getLocalOrJudge()) {
        moveRfgMenu->setTitle(tr("&Move exile to..."));
        aMoveRfgToTopLibrary->setText(tr("&Top of library"));
        aMoveRfgToBottomLibrary->setText(tr("&Bottom of library"));
        aMoveRfgToHand->setText(tr("&Hand"));
        aMoveRfgToGrave->setText(tr("&Graveyard"));
    }
}