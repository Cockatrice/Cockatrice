#include "hand_menu.h"

#include "../../abstract_game.h"
#include "../../zones/hand_zone.h"
#include "../player.h"
#include "../player_actions.h"

#include <QAction>
#include <QMenu>
#include <libcockatrice/settings/cache_settings.h>
#include <libcockatrice/settings/shortcuts_settings.h>

HandMenu::HandMenu(Player *_player, PlayerActions *actions, QWidget *parent) : TearOffMenu(parent), player(_player)
{
    if (player->getPlayerInfo()->local || player->getPlayerInfo()->judge) {
        aViewHand = new QAction(this);
        connect(aViewHand, &QAction::triggered, actions, &PlayerActions::actViewHand);
        addAction(aViewHand);

        aSortHand = new QAction(this);
        connect(aSortHand, &QAction::triggered, actions, &PlayerActions::actSortHand);
        addAction(aSortHand);
    }

    mRevealHand = addMenu(QString());
    connect(mRevealHand, &QMenu::aboutToShow, this, &HandMenu::populateRevealHandMenuWithActivePlayers);

    mRevealRandomHandCard = addMenu(QString());
    connect(mRevealRandomHandCard, &QMenu::aboutToShow, this,
            &HandMenu::populateRevealRandomHandCardMenuWithActivePlayers);

    addSeparator();

    aMulligan = new QAction(this);
    connect(aMulligan, &QAction::triggered, actions, &PlayerActions::actMulligan);
    addAction(aMulligan);

    addSeparator();

    mMoveHandMenu = addTearOffMenu(QString());

    if (player->getPlayerInfo()->local || player->getPlayerInfo()->judge) {
        aMoveHandToTopLibrary = new QAction(this);
        aMoveHandToTopLibrary->setData(QList<QVariant>() << "deck" << 0);
        aMoveHandToBottomLibrary = new QAction(this);
        aMoveHandToBottomLibrary->setData(QList<QVariant>() << "deck" << -1);
        aMoveHandToGrave = new QAction(this);
        aMoveHandToGrave->setData(QList<QVariant>() << "grave" << 0);
        aMoveHandToRfg = new QAction(this);
        aMoveHandToRfg->setData(QList<QVariant>() << "rfg" << 0);

        auto hand = player->getHandZone();

        connect(aMoveHandToTopLibrary, &QAction::triggered, hand, &HandZoneLogic::moveAllToZone);
        connect(aMoveHandToBottomLibrary, &QAction::triggered, hand, &HandZoneLogic::moveAllToZone);
        connect(aMoveHandToGrave, &QAction::triggered, hand, &HandZoneLogic::moveAllToZone);
        connect(aMoveHandToRfg, &QAction::triggered, hand, &HandZoneLogic::moveAllToZone);

        mMoveHandMenu->addAction(aMoveHandToTopLibrary);
        mMoveHandMenu->addAction(aMoveHandToBottomLibrary);
        mMoveHandMenu->addSeparator();
        mMoveHandMenu->addAction(aMoveHandToGrave);
        mMoveHandMenu->addSeparator();
        mMoveHandMenu->addAction(aMoveHandToRfg);
    }

    retranslateUi();
}

void HandMenu::retranslateUi()
{
    setTitle(tr("&Hand"));

    if (player->getPlayerInfo()->getLocalOrJudge()) {
        aViewHand->setText(tr("&View hand"));
        aSortHand->setText(tr("&Sort hand"));
        aMulligan->setText(tr("Take &mulligan"));

        mMoveHandMenu->setTitle(tr("&Move hand to..."));
        aMoveHandToTopLibrary->setText(tr("&Top of library"));
        aMoveHandToBottomLibrary->setText(tr("&Bottom of library"));
        aMoveHandToGrave->setText(tr("&Graveyard"));
        aMoveHandToRfg->setText(tr("&Exile"));

        mRevealHand->setTitle(tr("&Reveal hand to..."));
        mRevealRandomHandCard->setTitle(tr("Reveal r&andom card to..."));
    }
}

void HandMenu::setShortcutsActive()
{
    ShortcutsSettings &shortcuts = SettingsCache::instance().shortcuts();
    aViewHand->setShortcuts(shortcuts.getShortcut("Player/aViewHand"));
    aSortHand->setShortcuts(shortcuts.getShortcut("Player/aSortHand"));
    aMulligan->setShortcuts(shortcuts.getShortcut("Player/aMulligan"));
}

void HandMenu::setShortcutsInactive()
{
    aViewHand->setShortcut(QKeySequence());
    aSortHand->setShortcut(QKeySequence());
    aMulligan->setShortcut(QKeySequence());
}

void HandMenu::populateRevealHandMenuWithActivePlayers()
{
    mRevealHand->clear();

    QAction *allPlayers = mRevealHand->addAction(tr("&All players"));
    allPlayers->setData(-1);
    connect(allPlayers, &QAction::triggered, this, &HandMenu::onRevealHandTriggered);

    mRevealHand->addSeparator();

    const auto &players = player->getGame()->getPlayerManager()->getPlayers().values();
    for (auto *other : players) {
        if (other == player)
            continue;
        QAction *a = mRevealHand->addAction(other->getPlayerInfo()->getName());
        a->setData(other->getPlayerInfo()->getId());
        connect(a, &QAction::triggered, this, &HandMenu::onRevealHandTriggered);
    }
}

void HandMenu::populateRevealRandomHandCardMenuWithActivePlayers()
{
    mRevealRandomHandCard->clear();

    QAction *allPlayers = mRevealRandomHandCard->addAction(tr("&All players"));
    allPlayers->setData(-1);
    connect(allPlayers, &QAction::triggered, this, &HandMenu::onRevealRandomHandCardTriggered);

    mRevealRandomHandCard->addSeparator();

    const auto &players = player->getGame()->getPlayerManager()->getPlayers().values();
    for (auto *other : players) {
        if (other == player)
            continue;
        QAction *a = mRevealRandomHandCard->addAction(other->getPlayerInfo()->getName());
        a->setData(other->getPlayerInfo()->getId());
        connect(a, &QAction::triggered, this, &HandMenu::onRevealRandomHandCardTriggered);
    }
}

void HandMenu::onRevealHandTriggered()
{
    auto *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;

    const int targetId = action->data().toInt();
    player->getPlayerActions()->actRevealHand(targetId);
}

void HandMenu::onRevealRandomHandCardTriggered()
{
    auto *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;

    const int targetId = action->data().toInt();
    player->getPlayerActions()->actRevealRandomHandCard(targetId);
}
