#include "hand_menu.h"

#include "../../../client/settings/cache_settings.h"
#include "../../../client/settings/shortcuts_settings.h"
#include "../../abstract_game.h"
#include "../../zones/hand_zone.h"
#include "../player.h"
#include "../player_actions.h"

#include <QAction>
#include <QMenu>

HandMenu::HandMenu(Player *_player, PlayerActions *actions, QWidget *parent) : TearOffMenu(parent), player(_player)
{
    if (player->getPlayerInfo()->local || player->getPlayerInfo()->judge) {
        aViewHand = new QAction(this);
        connect(aViewHand, &QAction::triggered, actions, &PlayerActions::actViewHand);
        addAction(aViewHand);

        mSortHand = addMenu(QString());

        aSortHandByName = new QAction(this);
        aSortHandByName->setData(CardList::SortByName);
        aSortHandByType = new QAction(this);
        aSortHandByType->setData(CardList::SortByMainType);
        aSortHandByManaValue = new QAction(this);
        aSortHandByManaValue->setData(CardList::SortByManaValue);

        connect(aSortHandByType, &QAction::triggered, actions, &PlayerActions::actSortHand);
        connect(aSortHandByName, &QAction::triggered, actions, &PlayerActions::actSortHand);
        connect(aSortHandByManaValue, &QAction::triggered, actions, &PlayerActions::actSortHand);

        mSortHand->addAction(aSortHandByName);
        mSortHand->addAction(aSortHandByType);
        mSortHand->addAction(aSortHandByManaValue);
    }

    mRevealHand = addMenu(QString());
    connect(mRevealHand, &QMenu::aboutToShow, this, &HandMenu::populateRevealHandMenuWithActivePlayers);

    aRevealHandToAll = new QAction(this);
    aRevealHandToAll->setData(-1);
    connect(aRevealHandToAll, &QAction::triggered, this, &HandMenu::onRevealHandTriggered);

    mRevealRandomHandCard = addMenu(QString());
    connect(mRevealRandomHandCard, &QMenu::aboutToShow, this,
            &HandMenu::populateRevealRandomHandCardMenuWithActivePlayers);

    aRevealRandomHandCardToAll = new QAction(this);
    aRevealRandomHandCardToAll->setData(-1);
    connect(aRevealRandomHandCardToAll, &QAction::triggered, this, &HandMenu::onRevealRandomHandCardTriggered);

    // We still need to add these actions to menu here so that the shortcuts are active right away
    mRevealHand->addAction(aRevealHandToAll);
    mRevealRandomHandCard->addAction(aRevealRandomHandCardToAll);

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

        mSortHand->setTitle(tr("Sort hand by..."));
        aSortHandByName->setText(tr("Name"));
        aSortHandByType->setText(tr("Type"));
        aSortHandByManaValue->setText(tr("Mana Value"));

        aMulligan->setText(tr("Take &mulligan"));

        mMoveHandMenu->setTitle(tr("&Move hand to..."));
        aMoveHandToTopLibrary->setText(tr("&Top of library"));
        aMoveHandToBottomLibrary->setText(tr("&Bottom of library"));
        aMoveHandToGrave->setText(tr("&Graveyard"));
        aMoveHandToRfg->setText(tr("&Exile"));

        mRevealHand->setTitle(tr("&Reveal hand to..."));
        aRevealHandToAll->setText(tr("All players"));

        mRevealRandomHandCard->setTitle(tr("Reveal r&andom card to..."));
        aRevealRandomHandCardToAll->setText(tr("All players"));
    }
}

void HandMenu::setShortcutsActive()
{
    ShortcutsSettings &shortcuts = SettingsCache::instance().shortcuts();
    aViewHand->setShortcuts(shortcuts.getShortcut("Player/aViewHand"));
    aSortHandByName->setShortcuts(shortcuts.getShortcut("Player/aSortHandByName"));
    aSortHandByType->setShortcuts(shortcuts.getShortcut("Player/aSortHandByType"));
    aSortHandByManaValue->setShortcuts(shortcuts.getShortcut("Player/aSortHandByManaValue"));
    aMulligan->setShortcuts(shortcuts.getShortcut("Player/aMulligan"));
    aRevealHandToAll->setShortcuts(shortcuts.getShortcut("Player/aRevealHandToAll"));
    aRevealRandomHandCardToAll->setShortcuts(shortcuts.getShortcut("Player/aRevealRandomHandCardToAll"));
}

void HandMenu::setShortcutsInactive()
{
    aViewHand->setShortcut(QKeySequence());
    aSortHandByName->setShortcut(QKeySequence());
    aSortHandByType->setShortcut(QKeySequence());
    aSortHandByManaValue->setShortcut(QKeySequence());
    aMulligan->setShortcut(QKeySequence());
    aRevealHandToAll->setShortcut(QKeySequence());
    aRevealRandomHandCardToAll->setShortcut(QKeySequence());
}

void HandMenu::populateRevealHandMenuWithActivePlayers()
{
    mRevealHand->clear();

    mRevealHand->addAction(aRevealHandToAll);

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

    mRevealRandomHandCard->addAction(aRevealRandomHandCardToAll);

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
