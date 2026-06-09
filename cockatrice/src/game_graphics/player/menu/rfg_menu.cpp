#include "rfg_menu.h"

#include "../../game/player/player_actions.h"
#include "../../game/player/player_logic.h"
#include "../player_graphics_item.h"

#include <libcockatrice/utility/zone_names.h>

RfgMenu::RfgMenu(PlayerGraphicsItem *_player, QWidget *parent) : TearOffMenu(parent), player(_player)
{
    createMoveActions();
    createViewActions();

    addAction(aViewRfg);

    if (player->getLogic()->getPlayerInfo()->getLocalOrJudge()) {
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
    if (player->getLogic()->getPlayerInfo()->getLocalOrJudge()) {
        auto rfg = player->getLogic()->getRfgZone();

        aMoveRfgToTopLibrary = new QAction(this);
        aMoveRfgToTopLibrary->setData(QList<QVariant>() << ZoneNames::DECK << 0);
        aMoveRfgToBottomLibrary = new QAction(this);
        aMoveRfgToBottomLibrary->setData(QList<QVariant>() << ZoneNames::DECK << -1);
        aMoveRfgToHand = new QAction(this);
        aMoveRfgToHand->setData(QList<QVariant>() << ZoneNames::HAND << 0);
        aMoveRfgToGrave = new QAction(this);
        aMoveRfgToGrave->setData(QList<QVariant>() << ZoneNames::GRAVE << 0);

        connect(aMoveRfgToTopLibrary, &QAction::triggered, rfg, &PileZoneLogic::moveAllToZone);
        connect(aMoveRfgToBottomLibrary, &QAction::triggered, rfg, &PileZoneLogic::moveAllToZone);
        connect(aMoveRfgToHand, &QAction::triggered, rfg, &PileZoneLogic::moveAllToZone);
        connect(aMoveRfgToGrave, &QAction::triggered, rfg, &PileZoneLogic::moveAllToZone);
    }
}

void RfgMenu::createViewActions()
{
    PlayerActions *playerActions = player->getLogic()->getPlayerActions();

    aViewRfg = new QAction(this);
    connect(aViewRfg, &QAction::triggered, playerActions, &PlayerActions::actViewRfg);
}

void RfgMenu::retranslateUi()
{
    setTitle(tr("&Exile"));

    aViewRfg->setText(tr("&View exile"));

    if (player->getLogic()->getPlayerInfo()->getLocalOrJudge()) {
        moveRfgMenu->setTitle(tr("&Move exile to..."));
        aMoveRfgToTopLibrary->setText(tr("&Top of library"));
        aMoveRfgToBottomLibrary->setText(tr("&Bottom of library"));
        aMoveRfgToHand->setText(tr("&Hand"));
        aMoveRfgToGrave->setText(tr("&Graveyard"));
    }
}