#include "player.h"
#include "arrowitem.h"
#include "carddatabase.h"
#include "carditem.h"
#include "cardlist.h"
#include "cardzone.h"
#include "color.h"
#include "counter_general.h"
#include "deck_loader.h"
#include "dlg_create_token.h"
#include "gamescene.h"
#include "handcounter.h"
#include "handzone.h"
#include "main.h"
#include "pilezone.h"
#include "playertarget.h"
#include "settingscache.h"
#include "stackzone.h"
#include "tab_game.h"
#include "tablezone.h"
#include "thememanager.h"
#include "zoneviewwidget.h"
#include "zoneviewzone.h"
#include <QDebug>
#include <QMenu>
#include <QPainter>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QSignalMapper>

#include "pb/command_attach_card.pb.h"
#include "pb/command_change_zone_properties.pb.h"
#include "pb/command_create_token.pb.h"
#include "pb/command_draw_cards.pb.h"
#include "pb/command_flip_card.pb.h"
#include "pb/command_game_say.pb.h"
#include "pb/command_move_card.pb.h"
#include "pb/command_mulligan.pb.h"
#include "pb/command_reveal_cards.pb.h"
#include "pb/command_roll_die.pb.h"
#include "pb/command_set_card_attr.pb.h"
#include "pb/command_set_card_counter.pb.h"
#include "pb/command_shuffle.pb.h"
#include "pb/command_undo_draw.pb.h"
#include "pb/context_move_card.pb.h"
#include "pb/context_undo_draw.pb.h"
#include "pb/event_attach_card.pb.h"
#include "pb/event_change_zone_properties.pb.h"
#include "pb/event_create_arrow.pb.h"
#include "pb/event_create_counter.pb.h"
#include "pb/event_create_token.pb.h"
#include "pb/event_del_counter.pb.h"
#include "pb/event_delete_arrow.pb.h"
#include "pb/event_destroy_card.pb.h"
#include "pb/event_draw_cards.pb.h"
#include "pb/event_dump_zone.pb.h"
#include "pb/event_flip_card.pb.h"
#include "pb/event_game_say.pb.h"
#include "pb/event_move_card.pb.h"
#include "pb/event_reveal_cards.pb.h"
#include "pb/event_roll_die.pb.h"
#include "pb/event_set_card_attr.pb.h"
#include "pb/event_set_card_counter.pb.h"
#include "pb/event_set_counter.pb.h"
#include "pb/event_shuffle.pb.h"
#include "pb/event_stop_dump_zone.pb.h"
#include "pb/serverinfo_player.pb.h"
#include "pb/serverinfo_user.pb.h"
#include "pb/serverinfo_zone.pb.h"

PlayerArea::PlayerArea(QGraphicsItem *parentItem) : QObject(), QGraphicsItem(parentItem)
{
    setCacheMode(DeviceCoordinateCache);
    connect(themeManager, SIGNAL(themeChanged()), this, SLOT(updateBg()));
    updateBg();
}

void PlayerArea::updateBg()
{
    update();
}

void PlayerArea::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    painter->fillRect(bRect, themeManager->getPlayerBgBrush());
}

void PlayerArea::setSize(qreal width, qreal height)
{
    prepareGeometryChange();
    bRect = QRectF(0, 0, width, height);
}

Player::Player(const ServerInfo_User &info, int _id, bool _local, TabGame *_parent)
    : QObject(_parent), game(_parent), shortcutsActive(false), defaultNumberTopCards(1),
      defaultNumberTopCardsToPlaceBelow(1), lastTokenDestroy(true), lastTokenTableRow(0), id(_id), active(false),
      local(_local), mirrored(false), handVisible(false), conceded(false), dialogSemaphore(false), deck(nullptr)
{
    userInfo = new ServerInfo_User;
    userInfo->CopyFrom(info);

    connect(settingsCache, SIGNAL(horizontalHandChanged()), this, SLOT(rearrangeZones()));
    connect(settingsCache, SIGNAL(handJustificationChanged()), this, SLOT(rearrangeZones()));

    playerArea = new PlayerArea(this);

    playerTarget = new PlayerTarget(this, playerArea);
    qreal avatarMargin = (counterAreaWidth + CARD_HEIGHT + 15 - playerTarget->boundingRect().width()) / 2.0;
    playerTarget->setPos(QPointF(avatarMargin, avatarMargin));

    PileZone *deck = new PileZone(this, "deck", true, false, playerArea);
    QPointF base = QPointF(counterAreaWidth + (CARD_HEIGHT - CARD_WIDTH + 15) / 2.0,
                           10 + playerTarget->boundingRect().height() + 5 - (CARD_HEIGHT - CARD_WIDTH) / 2.0);
    deck->setPos(base);

    qreal h = deck->boundingRect().width() + 5;

    auto *handCounter = new HandCounter(playerArea);
    handCounter->setPos(base + QPointF(0, h + 10));
    qreal h2 = handCounter->boundingRect().height();

    PileZone *grave = new PileZone(this, "grave", false, true, playerArea);
    grave->setPos(base + QPointF(0, h + h2 + 10));

    PileZone *rfg = new PileZone(this, "rfg", false, true, playerArea);
    rfg->setPos(base + QPointF(0, 2 * h + h2 + 10));

    PileZone *sb = new PileZone(this, "sb", false, false, playerArea);
    sb->setVisible(false);

    table = new TableZone(this, this);
    connect(table, SIGNAL(sizeChanged()), this, SLOT(updateBoundingRect()));

    stack = new StackZone(this, (int)table->boundingRect().height(), this);

    hand = new HandZone(this, _local || (_parent->getSpectator() && _parent->getSpectatorsSeeEverything()),
                        (int)table->boundingRect().height(), this);
    connect(hand, SIGNAL(cardCountChanged()), handCounter, SLOT(updateNumber()));
    connect(handCounter, SIGNAL(showContextMenu(const QPoint &)), hand, SLOT(showContextMenu(const QPoint &)));

    updateBoundingRect();

    if (local) {
        connect(_parent, SIGNAL(playerAdded(Player *)), this, SLOT(addPlayer(Player *)));
        connect(_parent, SIGNAL(playerRemoved(Player *)), this, SLOT(removePlayer(Player *)));

        aMoveHandToTopLibrary = new QAction(this);
        aMoveHandToTopLibrary->setData(QList<QVariant>() << "deck" << 0);
        aMoveHandToBottomLibrary = new QAction(this);
        aMoveHandToBottomLibrary->setData(QList<QVariant>() << "deck" << -1);
        aMoveHandToGrave = new QAction(this);
        aMoveHandToGrave->setData(QList<QVariant>() << "grave" << 0);
        aMoveHandToRfg = new QAction(this);
        aMoveHandToRfg->setData(QList<QVariant>() << "rfg" << 0);

        connect(aMoveHandToTopLibrary, SIGNAL(triggered()), hand, SLOT(moveAllToZone()));
        connect(aMoveHandToBottomLibrary, SIGNAL(triggered()), hand, SLOT(moveAllToZone()));
        connect(aMoveHandToGrave, SIGNAL(triggered()), hand, SLOT(moveAllToZone()));
        connect(aMoveHandToRfg, SIGNAL(triggered()), hand, SLOT(moveAllToZone()));

        aMoveGraveToTopLibrary = new QAction(this);
        aMoveGraveToTopLibrary->setData(QList<QVariant>() << "deck" << 0);
        aMoveGraveToBottomLibrary = new QAction(this);
        aMoveGraveToBottomLibrary->setData(QList<QVariant>() << "deck" << -1);
        aMoveGraveToHand = new QAction(this);
        aMoveGraveToHand->setData(QList<QVariant>() << "hand" << 0);
        aMoveGraveToRfg = new QAction(this);
        aMoveGraveToRfg->setData(QList<QVariant>() << "rfg" << 0);

        connect(aMoveGraveToTopLibrary, SIGNAL(triggered()), grave, SLOT(moveAllToZone()));
        connect(aMoveGraveToBottomLibrary, SIGNAL(triggered()), grave, SLOT(moveAllToZone()));
        connect(aMoveGraveToHand, SIGNAL(triggered()), grave, SLOT(moveAllToZone()));
        connect(aMoveGraveToRfg, SIGNAL(triggered()), grave, SLOT(moveAllToZone()));

        aMoveRfgToTopLibrary = new QAction(this);
        aMoveRfgToTopLibrary->setData(QList<QVariant>() << "deck" << 0);
        aMoveRfgToBottomLibrary = new QAction(this);
        aMoveRfgToBottomLibrary->setData(QList<QVariant>() << "deck" << -1);
        aMoveRfgToHand = new QAction(this);
        aMoveRfgToHand->setData(QList<QVariant>() << "hand" << 0);
        aMoveRfgToGrave = new QAction(this);
        aMoveRfgToGrave->setData(QList<QVariant>() << "grave" << 0);

        connect(aMoveRfgToTopLibrary, SIGNAL(triggered()), rfg, SLOT(moveAllToZone()));
        connect(aMoveRfgToBottomLibrary, SIGNAL(triggered()), rfg, SLOT(moveAllToZone()));
        connect(aMoveRfgToHand, SIGNAL(triggered()), rfg, SLOT(moveAllToZone()));
        connect(aMoveRfgToGrave, SIGNAL(triggered()), rfg, SLOT(moveAllToZone()));

        aViewLibrary = new QAction(this);
        connect(aViewLibrary, SIGNAL(triggered()), this, SLOT(actViewLibrary()));
        aViewTopCards = new QAction(this);
        connect(aViewTopCards, SIGNAL(triggered()), this, SLOT(actViewTopCards()));
        aAlwaysRevealTopCard = new QAction(this);
        aAlwaysRevealTopCard->setCheckable(true);
        connect(aAlwaysRevealTopCard, SIGNAL(triggered()), this, SLOT(actAlwaysRevealTopCard()));
        aOpenDeckInDeckEditor = new QAction(this);
        aOpenDeckInDeckEditor->setEnabled(false);
        connect(aOpenDeckInDeckEditor, SIGNAL(triggered()), this, SLOT(actOpenDeckInDeckEditor()));
    }

    aViewGraveyard = new QAction(this);
    connect(aViewGraveyard, SIGNAL(triggered()), this, SLOT(actViewGraveyard()));

    aViewRfg = new QAction(this);
    connect(aViewRfg, SIGNAL(triggered()), this, SLOT(actViewRfg()));

    if (local) {
        aViewSideboard = new QAction(this);
        connect(aViewSideboard, SIGNAL(triggered()), this, SLOT(actViewSideboard()));

        aDrawCard = new QAction(this);
        connect(aDrawCard, SIGNAL(triggered()), this, SLOT(actDrawCard()));
        aDrawCards = new QAction(this);
        connect(aDrawCards, SIGNAL(triggered()), this, SLOT(actDrawCards()));
        aUndoDraw = new QAction(this);
        connect(aUndoDraw, SIGNAL(triggered()), this, SLOT(actUndoDraw()));
        aShuffle = new QAction(this);
        connect(aShuffle, SIGNAL(triggered()), this, SLOT(actShuffle()));
        aMulligan = new QAction(this);
        connect(aMulligan, SIGNAL(triggered()), this, SLOT(actMulligan()));
        aMoveTopToPlayFaceDown = new QAction(this);
        connect(aMoveTopToPlayFaceDown, SIGNAL(triggered()), this, SLOT(actMoveTopCardToPlayFaceDown()));
        aMoveTopCardToGraveyard = new QAction(this);
        connect(aMoveTopCardToGraveyard, SIGNAL(triggered()), this, SLOT(actMoveTopCardToGrave()));
        aMoveTopCardToExile = new QAction(this);
        connect(aMoveTopCardToExile, SIGNAL(triggered()), this, SLOT(actMoveTopCardToExile()));
        aMoveTopCardsToGraveyard = new QAction(this);
        connect(aMoveTopCardsToGraveyard, SIGNAL(triggered()), this, SLOT(actMoveTopCardsToGrave()));
        aMoveTopCardsToExile = new QAction(this);
        connect(aMoveTopCardsToExile, SIGNAL(triggered()), this, SLOT(actMoveTopCardsToExile()));
        aMoveTopCardToBottom = new QAction(this);
        connect(aMoveTopCardToBottom, SIGNAL(triggered()), this, SLOT(actMoveTopCardToBottom()));
        aMoveBottomCardToGrave = new QAction(this);
        connect(aMoveBottomCardToGrave, SIGNAL(triggered()), this, SLOT(actMoveBottomCardToGrave()));
    }

    playerMenu = new QMenu(QString());
    table->setMenu(playerMenu);

    if (local) {
        handMenu = playerMenu->addMenu(QString());
        playerLists.append(mRevealHand = handMenu->addMenu(QString()));
        playerLists.append(mRevealRandomHandCard = handMenu->addMenu(QString()));
        handMenu->addSeparator();
        handMenu->addAction(aMulligan);
        handMenu->addSeparator();
        moveHandMenu = handMenu->addMenu(QString());
        moveHandMenu->addAction(aMoveHandToTopLibrary);
        moveHandMenu->addAction(aMoveHandToBottomLibrary);
        moveHandMenu->addSeparator();
        moveHandMenu->addAction(aMoveHandToGrave);
        moveHandMenu->addSeparator();
        moveHandMenu->addAction(aMoveHandToRfg);
        hand->setMenu(handMenu);

        libraryMenu = playerMenu->addMenu(QString());
        libraryMenu->addAction(aDrawCard);
        libraryMenu->addAction(aDrawCards);
        libraryMenu->addAction(aUndoDraw);
        libraryMenu->addSeparator();
        libraryMenu->addAction(aShuffle);
        libraryMenu->addSeparator();
        libraryMenu->addAction(aViewLibrary);
        libraryMenu->addAction(aViewTopCards);
        libraryMenu->addSeparator();
        playerLists.append(mRevealLibrary = libraryMenu->addMenu(QString()));
        playerLists.append(mRevealTopCard = libraryMenu->addMenu(QString()));
        libraryMenu->addAction(aAlwaysRevealTopCard);
        libraryMenu->addSeparator();
        libraryMenu->addAction(aMoveTopToPlayFaceDown);
        libraryMenu->addAction(aMoveTopCardToBottom);
        libraryMenu->addAction(aMoveBottomCardToGrave);
        libraryMenu->addSeparator();
        libraryMenu->addAction(aMoveTopCardToGraveyard);
        libraryMenu->addAction(aMoveTopCardToExile);
        libraryMenu->addAction(aMoveTopCardsToGraveyard);
        libraryMenu->addAction(aMoveTopCardsToExile);
        libraryMenu->addSeparator();
        libraryMenu->addAction(aOpenDeckInDeckEditor);
        deck->setMenu(libraryMenu, aDrawCard);
    } else {
        handMenu = nullptr;
        libraryMenu = nullptr;
    }

    graveMenu = playerMenu->addMenu(QString());
    graveMenu->addAction(aViewGraveyard);

    if (local) {
        mRevealRandomGraveyardCard = graveMenu->addMenu(QString());
        QAction *newAction = mRevealRandomGraveyardCard->addAction(QString());
        newAction->setData(-1);
        connect(newAction, SIGNAL(triggered()), this, SLOT(actRevealRandomGraveyardCard()));
        allPlayersActions.append(newAction);
        mRevealRandomGraveyardCard->addSeparator();
    }
    grave->setMenu(graveMenu, aViewGraveyard);

    rfgMenu = playerMenu->addMenu(QString());
    rfgMenu->addAction(aViewRfg);
    rfg->setMenu(rfgMenu, aViewRfg);

    if (local) {
        graveMenu->addSeparator();
        moveGraveMenu = graveMenu->addMenu(QString());
        moveGraveMenu->addAction(aMoveGraveToTopLibrary);
        moveGraveMenu->addAction(aMoveGraveToBottomLibrary);
        moveGraveMenu->addSeparator();
        moveGraveMenu->addAction(aMoveGraveToHand);
        moveGraveMenu->addSeparator();
        moveGraveMenu->addAction(aMoveGraveToRfg);

        rfgMenu->addSeparator();
        moveRfgMenu = rfgMenu->addMenu(QString());
        moveRfgMenu->addAction(aMoveRfgToTopLibrary);
        moveRfgMenu->addAction(aMoveRfgToBottomLibrary);
        moveRfgMenu->addSeparator();
        moveRfgMenu->addAction(aMoveRfgToHand);
        moveRfgMenu->addSeparator();
        moveRfgMenu->addAction(aMoveRfgToGrave);

        sbMenu = playerMenu->addMenu(QString());
        sbMenu->addAction(aViewSideboard);
        sb->setMenu(sbMenu, aViewSideboard);

        aUntapAll = new QAction(this);
        connect(aUntapAll, SIGNAL(triggered()), this, SLOT(actUntapAll()));

        aRollDie = new QAction(this);
        connect(aRollDie, SIGNAL(triggered()), this, SLOT(actRollDie()));

        aCreateToken = new QAction(this);
        connect(aCreateToken, SIGNAL(triggered()), this, SLOT(actCreateToken()));

        aCreateAnotherToken = new QAction(this);
        connect(aCreateAnotherToken, SIGNAL(triggered()), this, SLOT(actCreateAnotherToken()));
        aCreateAnotherToken->setEnabled(false);

        createPredefinedTokenMenu = new QMenu(QString());

        playerMenu->addSeparator();
        countersMenu = playerMenu->addMenu(QString());
        playerMenu->addSeparator();
        playerMenu->addAction(aUntapAll);
        playerMenu->addSeparator();
        playerMenu->addAction(aRollDie);
        playerMenu->addSeparator();
        playerMenu->addAction(aCreateToken);
        playerMenu->addAction(aCreateAnotherToken);
        playerMenu->addMenu(createPredefinedTokenMenu);
        playerMenu->addSeparator();
        sayMenu = playerMenu->addMenu(QString());
        initSayMenu();

        aCardMenu = new QAction(this);
        playerMenu->addSeparator();
        playerMenu->addAction(aCardMenu);

        for (auto &playerList : playerLists) {
            QAction *newAction = playerList->addAction(QString());
            newAction->setData(-1);
            connect(newAction, SIGNAL(triggered()), this, SLOT(playerListActionTriggered()));
            allPlayersActions.append(newAction);
            playerList->addSeparator();
        }
    } else {
        countersMenu = nullptr;
        sbMenu = nullptr;
        aCreateAnotherToken = nullptr;
        createPredefinedTokenMenu = nullptr;
        aCardMenu = nullptr;
    }

    aTap = new QAction(this);
    aTap->setData(cmTap);
    connect(aTap, SIGNAL(triggered()), this, SLOT(cardMenuAction()));
    aDoesntUntap = new QAction(this);
    aDoesntUntap->setData(cmDoesntUntap);
    connect(aDoesntUntap, SIGNAL(triggered()), this, SLOT(cardMenuAction()));
    aAttach = new QAction(this);
    connect(aAttach, SIGNAL(triggered()), this, SLOT(actAttach()));
    aUnattach = new QAction(this);
    connect(aUnattach, SIGNAL(triggered()), this, SLOT(actUnattach()));
    aDrawArrow = new QAction(this);
    connect(aDrawArrow, SIGNAL(triggered()), this, SLOT(actDrawArrow()));
    aIncP = new QAction(this);
    connect(aIncP, SIGNAL(triggered()), this, SLOT(actIncP()));
    aDecP = new QAction(this);
    connect(aDecP, SIGNAL(triggered()), this, SLOT(actDecP()));
    aIncT = new QAction(this);
    connect(aIncT, SIGNAL(triggered()), this, SLOT(actIncT()));
    aDecT = new QAction(this);
    connect(aDecT, SIGNAL(triggered()), this, SLOT(actDecT()));
    aIncPT = new QAction(this);
    connect(aIncPT, SIGNAL(triggered()), this, SLOT(actIncPT()));
    aDecPT = new QAction(this);
    connect(aDecPT, SIGNAL(triggered()), this, SLOT(actDecPT()));
    aSetPT = new QAction(this);
    connect(aSetPT, SIGNAL(triggered()), this, SLOT(actSetPT()));
    aResetPT = new QAction(this);
    connect(aResetPT, SIGNAL(triggered()), this, SLOT(actResetPT()));
    aSetAnnotation = new QAction(this);
    connect(aSetAnnotation, SIGNAL(triggered()), this, SLOT(actSetAnnotation()));
    aFlip = new QAction(this);
    aFlip->setData(cmFlip);
    connect(aFlip, SIGNAL(triggered()), this, SLOT(cardMenuAction()));
    aPeek = new QAction(this);
    aPeek->setData(cmPeek);
    connect(aPeek, SIGNAL(triggered()), this, SLOT(cardMenuAction()));
    aClone = new QAction(this);
    aClone->setData(cmClone);
    connect(aClone, SIGNAL(triggered()), this, SLOT(cardMenuAction()));
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
    connect(aMoveToTopLibrary, SIGNAL(triggered()), this, SLOT(cardMenuAction()));
    connect(aMoveToBottomLibrary, SIGNAL(triggered()), this, SLOT(cardMenuAction()));
    connect(aMoveToXfromTopOfLibrary, SIGNAL(triggered()), this, SLOT(actMoveCardXCardsFromTop()));
    connect(aMoveToHand, SIGNAL(triggered()), this, SLOT(cardMenuAction()));
    connect(aMoveToGraveyard, SIGNAL(triggered()), this, SLOT(cardMenuAction()));
    connect(aMoveToExile, SIGNAL(triggered()), this, SLOT(cardMenuAction()));

    aPlay = new QAction(this);
    connect(aPlay, SIGNAL(triggered()), this, SLOT(actPlay()));
    aHide = new QAction(this);
    connect(aHide, SIGNAL(triggered()), this, SLOT(actHide()));
    aPlayFacedown = new QAction(this);
    connect(aPlayFacedown, SIGNAL(triggered()), this, SLOT(actPlayFacedown()));

    for (int i = 0; i < 3; ++i) {
        auto *tempAddCounter = new QAction(this);
        tempAddCounter->setData(9 + i * 1000);
        auto *tempRemoveCounter = new QAction(this);
        tempRemoveCounter->setData(10 + i * 1000);
        auto *tempSetCounter = new QAction(this);
        tempSetCounter->setData(11 + i * 1000);
        aAddCounter.append(tempAddCounter);
        aRemoveCounter.append(tempRemoveCounter);
        aSetCounter.append(tempSetCounter);
        connect(tempAddCounter, SIGNAL(triggered()), this, SLOT(actCardCounterTrigger()));
        connect(tempRemoveCounter, SIGNAL(triggered()), this, SLOT(actCardCounterTrigger()));
        connect(tempSetCounter, SIGNAL(triggered()), this, SLOT(actCardCounterTrigger()));
    }

    const QList<Player *> &players = game->getPlayers().values();
    for (auto player : players)
        addPlayer(player);

    rearrangeZones();
    retranslateUi();
    connect(&settingsCache->shortcuts(), SIGNAL(shortCutChanged()), this, SLOT(refreshShortcuts()));
    refreshShortcuts();
}

Player::~Player()
{
    qDebug() << "Player destructor:" << getName();

    static_cast<GameScene *>(scene())->removePlayer(this);

    clear();
    QMapIterator<QString, CardZone *> i(zones);
    while (i.hasNext())
        delete i.next().value();
    zones.clear();

    delete playerMenu;
    delete userInfo;
}

void Player::clear()
{
    clearArrows();

    QMapIterator<QString, CardZone *> i(zones);
    while (i.hasNext()) {
        i.next().value()->clearContents();
    }

    clearCounters();
}

void Player::addPlayer(Player *player)
{
    if (player == nullptr || player == this) {
        return;
    }

    for (auto &playerList : playerLists) {
        QAction *newAction = playerList->addAction(player->getName());
        newAction->setData(player->getId());
        connect(newAction, SIGNAL(triggered()), this, SLOT(playerListActionTriggered()));
    }
}

void Player::removePlayer(Player *player)
{
    if (player == nullptr) {
        return;
    }

    for (auto &playerList : playerLists) {
        QList<QAction *> actionList = playerList->actions();
        for (auto &j : actionList)
            if (j->data().toInt() == player->getId()) {
                playerList->removeAction(j);
                j->deleteLater();
            }
    }
}

void Player::playerListActionTriggered()
{
    auto *action = static_cast<QAction *>(sender());
    auto *menu = static_cast<QMenu *>(action->parentWidget());

    Command_RevealCards cmd;
    const int otherPlayerId = action->data().toInt();
    if (otherPlayerId != -1) {
        cmd.set_player_id(otherPlayerId);
    }

    if (menu == mRevealLibrary) {
        cmd.set_zone_name("deck");
    } else if (menu == mRevealTopCard) {
        int decksize = zones.value("deck")->getCards().size();
        bool ok;
        int number = QInputDialog::getInt(nullptr, tr("Reveal top cards of library"),
                                          tr("Number of cards: (max. %1)").arg(decksize), defaultNumberTopCards, 1,
                                          decksize, 1, &ok);
        if (ok) {
            cmd.set_zone_name("deck");
            cmd.set_top_cards(number);
            // backward compatibility: servers before #1051 only permits to reveal the first card
            cmd.set_card_id(0);
        }

    } else if (menu == mRevealHand) {
        cmd.set_zone_name("hand");
    } else if (menu == mRevealRandomHandCard) {
        cmd.set_zone_name("hand");
        cmd.set_card_id(RANDOM_CARD_FROM_ZONE);
    } else {
        return;
    }

    sendGameCommand(cmd);
}

void Player::rearrangeZones()
{
    QPointF base = QPointF(CARD_HEIGHT + counterAreaWidth + 15, 0);
    if (settingsCache->getHorizontalHand()) {
        if (mirrored) {
            if (hand->contentsKnown()) {
                handVisible = true;
                hand->setPos(base);
                base += QPointF(0, hand->boundingRect().height());
            } else
                handVisible = false;

            stack->setPos(base);
            base += QPointF(stack->boundingRect().width(), 0);

            table->setPos(base);
        } else {
            stack->setPos(base);

            table->setPos(base.x() + stack->boundingRect().width(), 0);
            base += QPointF(0, table->boundingRect().height());

            if (hand->contentsKnown()) {
                handVisible = true;
                hand->setPos(base);
            } else
                handVisible = false;
        }
        hand->setWidth(table->getWidth() + stack->boundingRect().width());
    } else {
        handVisible = true;

        hand->setPos(base);
        base += QPointF(hand->boundingRect().width(), 0);

        stack->setPos(base);
        base += QPointF(stack->boundingRect().width(), 0);

        table->setPos(base);
    }
    hand->setVisible(handVisible);
    hand->updateOrientation();
    table->reorganizeCards();
    updateBoundingRect();
    rearrangeCounters();
}

void Player::updateZones()
{
    table->reorganizeCards();
}

void Player::updateBoundingRect()
{
    prepareGeometryChange();
    qreal width = CARD_HEIGHT + 15 + counterAreaWidth + stack->boundingRect().width();
    if (settingsCache->getHorizontalHand()) {
        qreal handHeight = handVisible ? hand->boundingRect().height() : 0;
        bRect = QRectF(0, 0, width + table->boundingRect().width(), table->boundingRect().height() + handHeight);
    } else {
        bRect = QRectF(0, 0, width + hand->boundingRect().width() + table->boundingRect().width(),
                       table->boundingRect().height());
    }
    playerArea->setSize(CARD_HEIGHT + counterAreaWidth + 15, bRect.height());

    emit sizeChanged();
}

void Player::retranslateUi()
{
    aViewGraveyard->setText(tr("&View graveyard"));
    aViewRfg->setText(tr("&View exile"));

    playerMenu->setTitle(tr("Player \"%1\"").arg(QString::fromStdString(userInfo->name())));
    graveMenu->setTitle(tr("&Graveyard"));
    rfgMenu->setTitle(tr("&Exile"));

    if (local) {
        moveHandMenu->setTitle(tr("&Move hand to..."));
        aMoveHandToTopLibrary->setText(tr("&Top of library"));
        aMoveHandToBottomLibrary->setText(tr("&Bottom of library"));
        aMoveHandToGrave->setText(tr("&Graveyard"));
        aMoveHandToRfg->setText(tr("&Exile"));

        moveGraveMenu->setTitle(tr("&Move graveyard to..."));
        aMoveGraveToTopLibrary->setText(tr("&Top of library"));
        aMoveGraveToBottomLibrary->setText(tr("&Bottom of library"));
        aMoveGraveToHand->setText(tr("&Hand"));
        aMoveGraveToRfg->setText(tr("&Exile"));

        moveRfgMenu->setTitle(tr("&Move exile to..."));
        aMoveRfgToTopLibrary->setText(tr("&Top of library"));
        aMoveRfgToBottomLibrary->setText(tr("&Bottom of library"));
        aMoveRfgToHand->setText(tr("&Hand"));
        aMoveRfgToGrave->setText(tr("&Graveyard"));

        aViewLibrary->setText(tr("&View library"));
        aViewTopCards->setText(tr("View &top cards of library..."));
        mRevealLibrary->setTitle(tr("Reveal &library to..."));
        mRevealTopCard->setTitle(tr("Reveal t&op cards to..."));
        aAlwaysRevealTopCard->setText(tr("&Always reveal top card"));
        aOpenDeckInDeckEditor->setText(tr("O&pen deck in deck editor"));
        aViewSideboard->setText(tr("&View sideboard"));
        aDrawCard->setText(tr("&Draw card"));
        aDrawCards->setText(tr("D&raw cards..."));
        aUndoDraw->setText(tr("&Undo last draw"));
        aMulligan->setText(tr("Take &mulligan"));
        aShuffle->setText(tr("&Shuffle"));
        aMoveTopToPlayFaceDown->setText(tr("Play top card &face down"));
        aMoveTopCardToGraveyard->setText(tr("Move top card to grave&yard"));
        aMoveTopCardToExile->setText(tr("Move top card to e&xile"));
        aMoveTopCardsToGraveyard->setText(tr("Move top cards to &graveyard..."));
        aMoveTopCardsToExile->setText(tr("Move top cards to &exile..."));
        aMoveTopCardToBottom->setText(tr("Put top card on &bottom"));
        aMoveBottomCardToGrave->setText(tr("Put bottom card &in graveyard"));

        handMenu->setTitle(tr("&Hand"));
        mRevealHand->setTitle(tr("&Reveal hand to..."));
        mRevealRandomHandCard->setTitle(tr("Reveal r&andom card to..."));
        mRevealRandomGraveyardCard->setTitle(tr("Reveal random card to..."));
        sbMenu->setTitle(tr("&Sideboard"));
        libraryMenu->setTitle(tr("&Library"));
        countersMenu->setTitle(tr("&Counters"));

        aUntapAll->setText(tr("&Untap all permanents"));
        aRollDie->setText(tr("R&oll die..."));
        aCreateToken->setText(tr("&Create token..."));
        aCreateAnotherToken->setText(tr("C&reate another token"));
        createPredefinedTokenMenu->setTitle(tr("Cr&eate predefined token"));
        sayMenu->setTitle(tr("S&ay"));

        QMapIterator<int, AbstractCounter *> counterIterator(counters);
        while (counterIterator.hasNext())
            counterIterator.next().value()->retranslateUi();

        aCardMenu->setText(tr("C&ard"));

        for (auto &allPlayersAction : allPlayersActions)
            allPlayersAction->setText(tr("&All players"));
    }

    aPlay->setText(tr("&Play"));
    aHide->setText(tr("&Hide"));
    aPlayFacedown->setText(tr("Play &Face Down"));
    //: Turn sideways or back again
    aTap->setText(tr("&Tap / Untap"));
    aDoesntUntap->setText(tr("Toggle &normal untapping"));
    //: Turn face up/face down
    aFlip->setText(tr("T&urn Over")); // Only the user facing names in client got renamed to "turn over"
                                      // All code and proto bits are still unchanged (flip) for compatibility reasons
                                      // A protocol rewrite with v3 could incorporate that, see #3100
    aPeek->setText(tr("&Peek at card face"));
    aClone->setText(tr("&Clone"));
    aAttach->setText(tr("Attac&h to card..."));
    aUnattach->setText(tr("Unattac&h"));
    aDrawArrow->setText(tr("&Draw arrow..."));
    aIncP->setText(tr("&Increase power"));
    aDecP->setText(tr("&Decrease power"));
    aIncT->setText(tr("I&ncrease toughness"));
    aDecT->setText(tr("D&ecrease toughness"));
    aIncPT->setText(tr("In&crease power and toughness"));
    aDecPT->setText(tr("Dec&rease power and toughness"));
    aSetPT->setText(tr("Set &power and toughness..."));
    aResetPT->setText(tr("Reset p&ower and toughness"));
    aSetAnnotation->setText(tr("&Set annotation..."));

    QStringList counterColors;
    counterColors.append(tr("Red"));
    counterColors.append(tr("Yellow"));
    counterColors.append(tr("Green"));

    for (int i = 0; i < aAddCounter.size(); ++i) {
        aAddCounter[i]->setText(tr("&Add counter (%1)").arg(counterColors[i]));
    }
    for (int i = 0; i < aRemoveCounter.size(); ++i) {
        aRemoveCounter[i]->setText(tr("&Remove counter (%1)").arg(counterColors[i]));
    }
    for (int i = 0; i < aSetCounter.size(); ++i) {
        aSetCounter[i]->setText(tr("&Set counters (%1)...").arg(counterColors[i]));
    }

    aMoveToTopLibrary->setText(tr("&Top of library"));
    aMoveToXfromTopOfLibrary->setText(tr("X cards from the top of library..."));
    aMoveToBottomLibrary->setText(tr("&Bottom of library in random order"));
    aMoveToHand->setText(tr("&Hand"));
    aMoveToGraveyard->setText(tr("&Graveyard"));
    aMoveToExile->setText(tr("&Exile"));

    QMapIterator<QString, CardZone *> zoneIterator(zones);
    while (zoneIterator.hasNext()) {
        zoneIterator.next().value()->retranslateUi();
    }
}

void Player::setShortcutsActive()
{
    shortcutsActive = true;
    ShortcutsSettings &shortcuts = settingsCache->shortcuts();

    aPlay->setShortcuts(shortcuts.getShortcut("Player/aPlay"));
    aTap->setShortcuts(shortcuts.getShortcut("Player/aTap"));
    aDoesntUntap->setShortcuts(shortcuts.getShortcut("Player/aDoesntUntap"));
    aFlip->setShortcuts(shortcuts.getShortcut("Player/aFlip"));
    aPeek->setShortcuts(shortcuts.getShortcut("Player/aPeek"));
    aClone->setShortcuts(shortcuts.getShortcut("Player/aClone"));
    aAttach->setShortcuts(shortcuts.getShortcut("Player/aAttach"));
    aUnattach->setShortcuts(shortcuts.getShortcut("Player/aUnattach"));
    aDrawArrow->setShortcuts(shortcuts.getShortcut("Player/aDrawArrow"));
    aIncP->setShortcuts(shortcuts.getShortcut("Player/aIncP"));
    aDecP->setShortcuts(shortcuts.getShortcut("Player/aDecP"));
    aIncT->setShortcuts(shortcuts.getShortcut("Player/aIncT"));
    aDecT->setShortcuts(shortcuts.getShortcut("Player/aDecT"));
    aIncPT->setShortcuts(shortcuts.getShortcut("Player/aIncPT"));
    aDecPT->setShortcuts(shortcuts.getShortcut("Player/aDecPT"));
    aSetPT->setShortcuts(shortcuts.getShortcut("Player/aSetPT"));
    aResetPT->setShortcuts(shortcuts.getShortcut("Player/aResetPT"));
    aSetAnnotation->setShortcuts(shortcuts.getShortcut("Player/aSetAnnotation"));
    aMoveToTopLibrary->setShortcuts(shortcuts.getShortcut("Player/aMoveToTopLibrary"));
    aMoveToBottomLibrary->setShortcuts(shortcuts.getShortcut("Player/aMoveToBottomLibrary"));
    aMoveToHand->setShortcuts(shortcuts.getShortcut("Player/aMoveToHand"));
    aMoveToGraveyard->setShortcuts(shortcuts.getShortcut("Player/aMoveToGraveyard"));
    aMoveToExile->setShortcuts(shortcuts.getShortcut("Player/aMoveToExile"));

    QList<QKeySequence> addCCShortCuts;
    addCCShortCuts.append(shortcuts.getSingleShortcut("Player/aCCRed"));
    addCCShortCuts.append(shortcuts.getSingleShortcut("Player/aCCYellow"));
    addCCShortCuts.append(shortcuts.getSingleShortcut("Player/aCCGreen"));

    QList<QKeySequence> removeCCShortCuts;
    removeCCShortCuts.append(shortcuts.getSingleShortcut("Player/aRCRed"));
    removeCCShortCuts.append(shortcuts.getSingleShortcut("Player/aRCYellow"));
    removeCCShortCuts.append(shortcuts.getSingleShortcut("Player/aRCGreen"));

    QList<QKeySequence> setCCShortCuts;
    setCCShortCuts.append(shortcuts.getSingleShortcut("Player/aSCRed"));
    setCCShortCuts.append(shortcuts.getSingleShortcut("Player/aSCYellow"));
    setCCShortCuts.append(shortcuts.getSingleShortcut("Player/aSCGreen"));

    for (int i = 0; i < aAddCounter.size(); ++i) {
        aAddCounter[i]->setShortcut(addCCShortCuts.at(i));
    }
    for (int i = 0; i < aRemoveCounter.size(); ++i) {
        aRemoveCounter[i]->setShortcut(removeCCShortCuts.at(i));
    }
    for (int i = 0; i < aSetCounter.size(); ++i) {
        aSetCounter[i]->setShortcut(setCCShortCuts.at(i));
    }

    QMapIterator<int, AbstractCounter *> counterIterator(counters);
    while (counterIterator.hasNext()) {
        counterIterator.next().value()->setShortcutsActive();
    }

    aViewSideboard->setShortcut(shortcuts.getSingleShortcut("Player/aViewSideboard"));
    aViewLibrary->setShortcut(shortcuts.getSingleShortcut("Player/aViewLibrary"));
    aViewTopCards->setShortcut(shortcuts.getSingleShortcut("Player/aViewTopCards"));
    aViewGraveyard->setShortcut(shortcuts.getSingleShortcut("Player/aViewGraveyard"));
    aDrawCard->setShortcut(shortcuts.getSingleShortcut("Player/aDrawCard"));
    aDrawCards->setShortcut(shortcuts.getSingleShortcut("Player/aDrawCards"));
    aUndoDraw->setShortcut(shortcuts.getSingleShortcut("Player/aUndoDraw"));
    aMulligan->setShortcut(shortcuts.getSingleShortcut("Player/aMulligan"));
    aShuffle->setShortcut(shortcuts.getSingleShortcut("Player/aShuffle"));
    aUntapAll->setShortcut(shortcuts.getSingleShortcut("Player/aUntapAll"));
    aRollDie->setShortcut(shortcuts.getSingleShortcut("Player/aRollDie"));
    aCreateToken->setShortcut(shortcuts.getSingleShortcut("Player/aCreateToken"));
    aCreateAnotherToken->setShortcut(shortcuts.getSingleShortcut("Player/aCreateAnotherToken"));
    aAlwaysRevealTopCard->setShortcut(shortcuts.getSingleShortcut("Player/aAlwaysRevealTopCard"));
    aMoveTopToPlayFaceDown->setShortcut(shortcuts.getSingleShortcut("Player/aMoveTopToPlayFaceDown"));
    aMoveTopCardToGraveyard->setShortcut(shortcuts.getSingleShortcut("Player/aMoveTopCardToGraveyard"));
    aMoveTopCardsToGraveyard->setShortcut(shortcuts.getSingleShortcut("Player/aMoveTopCardsToGraveyard"));
    aMoveTopCardToExile->setShortcut(shortcuts.getSingleShortcut("Player/aMoveTopCardToExile"));
    aMoveTopCardsToExile->setShortcut(shortcuts.getSingleShortcut("Player/aMoveTopCardsToExile"));
}

void Player::setShortcutsInactive()
{
    shortcutsActive = false;

    aViewSideboard->setShortcut(QKeySequence());
    aViewLibrary->setShortcut(QKeySequence());
    aViewTopCards->setShortcut(QKeySequence());
    aViewGraveyard->setShortcut(QKeySequence());
    aDrawCard->setShortcut(QKeySequence());
    aDrawCards->setShortcut(QKeySequence());
    aUndoDraw->setShortcut(QKeySequence());
    aMulligan->setShortcut(QKeySequence());
    aShuffle->setShortcut(QKeySequence());
    aUntapAll->setShortcut(QKeySequence());
    aRollDie->setShortcut(QKeySequence());
    aCreateToken->setShortcut(QKeySequence());
    aCreateAnotherToken->setShortcut(QKeySequence());
    aAlwaysRevealTopCard->setShortcut(QKeySequence());
    aMoveTopToPlayFaceDown->setShortcut(QKeySequence());
    aMoveTopCardToGraveyard->setShortcut(QKeySequence());
    aMoveTopCardsToGraveyard->setShortcut(QKeySequence());
    aMoveTopCardToExile->setShortcut(QKeySequence());
    aMoveTopCardsToExile->setShortcut(QKeySequence());

    QMapIterator<int, AbstractCounter *> counterIterator(counters);
    while (counterIterator.hasNext()) {
        counterIterator.next().value()->setShortcutsInactive();
    }
}

void Player::initSayMenu()
{
    sayMenu->clear();

    int count = settingsCache->messages().getCount();

    for (int i = 0; i < count; ++i) {
        QAction *newAction = new QAction(settingsCache->messages().getMessageAt(i), this);
        if (i <= 10) {
            newAction->setShortcut(QKeySequence("Ctrl+" + QString::number((i + 1) % 10)));
        }
        connect(newAction, SIGNAL(triggered()), this, SLOT(actSayMessage()));
        sayMenu->addAction(newAction);
    }
}

void Player::setDeck(const DeckLoader &_deck)
{
    deck = new DeckLoader(_deck);
    aOpenDeckInDeckEditor->setEnabled(deck);

    createPredefinedTokenMenu->clear();
    predefinedTokens.clear();
    InnerDecklistNode *tokenZone = dynamic_cast<InnerDecklistNode *>(deck->getRoot()->findChild(DECK_ZONE_TOKENS));

    if (tokenZone)
        for (int i = 0; i < tokenZone->size(); ++i) {
            const QString tokenName = tokenZone->at(i)->getName();
            predefinedTokens.append(tokenName);
            QAction *a = createPredefinedTokenMenu->addAction(tokenName);
            if (i < 10) {
                a->setShortcut(QKeySequence("Alt+" + QString::number((i + 1) % 10)));
            }
            connect(a, SIGNAL(triggered()), this, SLOT(actCreatePredefinedToken()));
        }
}

void Player::actViewLibrary()
{
    static_cast<GameScene *>(scene())->toggleZoneView(this, "deck", -1);
}

void Player::actViewTopCards()
{
    bool ok;
    int number = QInputDialog::getInt(nullptr, tr("View top cards of library"), tr("Number of cards:"),
                                      defaultNumberTopCards, 1, 2000000000, 1, &ok);
    if (ok) {
        defaultNumberTopCards = number;
        static_cast<GameScene *>(scene())->toggleZoneView(this, "deck", number);
    }
}

void Player::actAlwaysRevealTopCard()
{
    Command_ChangeZoneProperties cmd;
    cmd.set_zone_name("deck");
    cmd.set_always_reveal_top_card(aAlwaysRevealTopCard->isChecked());

    sendGameCommand(cmd);
}

void Player::actOpenDeckInDeckEditor()
{
    emit openDeckEditor(deck);
}

void Player::actViewGraveyard()
{
    dynamic_cast<GameScene *>(scene())->toggleZoneView(this, "grave", -1);
}

void Player::actRevealRandomGraveyardCard()
{
    Command_RevealCards cmd;
    auto *action = dynamic_cast<QAction *>(sender());
    const int otherPlayerId = action->data().toInt();
    if (otherPlayerId != -1) {
        cmd.set_player_id(otherPlayerId);
    }
    cmd.set_zone_name("grave");
    cmd.set_card_id(RANDOM_CARD_FROM_ZONE);
    sendGameCommand(cmd);
}

void Player::actViewRfg()
{
    static_cast<GameScene *>(scene())->toggleZoneView(this, "rfg", -1);
}

void Player::actViewSideboard()
{
    static_cast<GameScene *>(scene())->toggleZoneView(this, "sb", -1);
}

void Player::actShuffle()
{
    sendGameCommand(Command_Shuffle());
}

void Player::actDrawCard()
{
    Command_DrawCards cmd;
    cmd.set_number(1);
    sendGameCommand(cmd);
}

void Player::actMulligan()
{
    sendGameCommand(Command_Mulligan());
}

void Player::actDrawCards()
{
    int number = QInputDialog::getInt(nullptr, tr("Draw cards"), tr("Number:"));
    if (number) {
        Command_DrawCards cmd;
        cmd.set_number(static_cast<google::protobuf::uint32>(number));
        sendGameCommand(cmd);
    }
}

void Player::actUndoDraw()
{
    sendGameCommand(Command_UndoDraw());
}

void Player::actMoveTopCardToGrave()
{
    if (zones.value("deck")->getCards().empty()) {
        return;
    }

    Command_MoveCard cmd;
    cmd.set_start_zone("deck");
    cmd.mutable_cards_to_move()->add_card()->set_card_id(0);
    cmd.set_target_player_id(getId());
    cmd.set_target_zone("grave");
    cmd.set_x(0);
    cmd.set_y(0);

    sendGameCommand(cmd);
}

void Player::actMoveTopCardToExile()
{
    if (zones.value("deck")->getCards().empty()) {
        return;
    }

    Command_MoveCard cmd;
    cmd.set_start_zone("deck");
    cmd.mutable_cards_to_move()->add_card()->set_card_id(0);
    cmd.set_target_player_id(getId());
    cmd.set_target_zone("rfg");
    cmd.set_x(0);
    cmd.set_y(0);

    sendGameCommand(cmd);
}

void Player::actMoveTopCardsToGrave()
{
    int number = QInputDialog::getInt(nullptr, tr("Move top cards to grave"), tr("Number:"));
    if (!number) {
        return;
    }

    const int maxCards = zones.value("deck")->getCards().size();
    if (number > maxCards) {
        number = maxCards;
    }

    Command_MoveCard cmd;
    cmd.set_start_zone("deck");
    cmd.set_target_player_id(getId());
    cmd.set_target_zone("grave");
    cmd.set_x(0);
    cmd.set_y(0);

    for (int i = number - 1; i >= 0; --i) {
        cmd.mutable_cards_to_move()->add_card()->set_card_id(i);
    }

    sendGameCommand(cmd);
}

void Player::actMoveTopCardsToExile()
{
    int number = QInputDialog::getInt(nullptr, tr("Move top cards to exile"), tr("Number:"));
    if (!number) {
        return;
    }

    const int maxCards = zones.value("deck")->getCards().size();
    if (number > maxCards) {
        number = maxCards;
    }

    Command_MoveCard cmd;
    cmd.set_start_zone("deck");
    cmd.set_target_player_id(getId());
    cmd.set_target_zone("rfg");
    cmd.set_x(0);
    cmd.set_y(0);

    for (int i = number - 1; i >= 0; --i) {
        cmd.mutable_cards_to_move()->add_card()->set_card_id(i);
    }

    sendGameCommand(cmd);
}

void Player::actMoveTopCardToBottom()
{
    Command_MoveCard cmd;
    cmd.set_start_zone("deck");
    cmd.mutable_cards_to_move()->add_card()->set_card_id(0);
    cmd.set_target_player_id(getId());
    cmd.set_target_zone("deck");
    cmd.set_x(-1);
    cmd.set_y(0);

    sendGameCommand(cmd);
}

void Player::actMoveTopCardToPlayFaceDown()
{
    Command_MoveCard cmd;
    cmd.set_start_zone("deck");
    CardToMove *cardToMove = cmd.mutable_cards_to_move()->add_card();
    cardToMove->set_card_id(0);
    cardToMove->set_face_down(true);
    cmd.set_target_player_id(getId());
    cmd.set_target_zone("table");
    cmd.set_x(-1);
    cmd.set_y(0);

    sendGameCommand(cmd);
}

void Player::actMoveBottomCardToGrave()
{
    CardZone *zone = zones.value("deck");
    Command_MoveCard cmd;
    cmd.set_start_zone("deck");
    cmd.mutable_cards_to_move()->add_card()->set_card_id(zone->getCards().size() - 1);
    cmd.set_target_player_id(getId());
    cmd.set_target_zone("grave");
    cmd.set_x(0);
    cmd.set_y(0);

    sendGameCommand(cmd);
}

void Player::actUntapAll()
{
    Command_SetCardAttr cmd;
    cmd.set_zone("table");
    cmd.set_attribute(AttrTapped);
    cmd.set_attr_value("0");

    sendGameCommand(cmd);
}

void Player::actRollDie()
{
    bool ok;
    int sides = QInputDialog::getInt(static_cast<QWidget *>(parent()), tr("Roll die"), tr("Number of sides:"), 20, 2,
                                     1000, 1, &ok);
    if (ok) {
        Command_RollDie cmd;
        cmd.set_sides(static_cast<google::protobuf::uint32>(sides));
        sendGameCommand(cmd);
    }
}

void Player::actCreateToken()
{
    DlgCreateToken dlg(predefinedTokens);
    if (!dlg.exec()) {
        return;
    }

    lastTokenName = dlg.getName();
    lastTokenPT = dlg.getPT();
    CardInfoPtr correctedCard = db->getCardBySimpleName(lastTokenName);
    if (correctedCard) {
        lastTokenName = correctedCard->getName();
        lastTokenTableRow = TableZone::clampValidTableRow(2 - correctedCard->getTableRow());
        if (lastTokenPT.isEmpty()) {
            lastTokenPT = correctedCard->getPowTough();
        }
    }
    lastTokenColor = dlg.getColor();
    lastTokenAnnotation = dlg.getAnnotation();
    lastTokenDestroy = dlg.getDestroy();
    aCreateAnotherToken->setEnabled(true);
    aCreateAnotherToken->setText(tr("C&reate another %1 token").arg(lastTokenName));
    actCreateAnotherToken();
}

void Player::actCreateAnotherToken()
{
    if (lastTokenName.isEmpty()) {
        return;
    }

    Command_CreateToken cmd;
    cmd.set_zone("table");
    cmd.set_card_name(lastTokenName.toStdString());
    cmd.set_color(lastTokenColor.toStdString());
    cmd.set_pt(lastTokenPT.toStdString());
    cmd.set_annotation(lastTokenAnnotation.toStdString());
    cmd.set_destroy_on_zone_change(lastTokenDestroy);
    cmd.set_x(-1);
    cmd.set_y(lastTokenTableRow);

    sendGameCommand(cmd);
}

void Player::actCreatePredefinedToken()
{
    auto *action = static_cast<QAction *>(sender());
    CardInfoPtr cardInfo = db->getCard(action->text());
    if (!cardInfo) {
        return;
    }

    setLastToken(cardInfo);

    actCreateAnotherToken();
}

void Player::actCreateRelatedCard()
{
    CardItem *sourceCard = game->getActiveCard();
    if (!sourceCard) {
        return;
    }
    auto *action = static_cast<QAction *>(sender());
    // If there is a better way of passing a CardRelation through a QAction, please add it here.
    QList<CardRelation *> relatedCards = QList<CardRelation *>();
    relatedCards.append(sourceCard->getInfo()->getRelatedCards());
    relatedCards.append(sourceCard->getInfo()->getReverseRelatedCards2Me());
    CardRelation *cardRelation = relatedCards.at(action->data().toInt());

    /*
     * If we make a token via "Token: TokenName"
     * then let's allow it to be created via "create another token"
     */
    if (createRelatedFromRelation(sourceCard, cardRelation) && cardRelation->getCanCreateAnother()) {
        CardInfoPtr cardInfo = db->getCard(cardRelation->getName());
        setLastToken(cardInfo);
    }
}

void Player::actCreateAllRelatedCards()
{
    CardItem *sourceCard = game->getActiveCard();
    if (!sourceCard) {
        return;
    }

    QList<CardRelation *> relatedCards = sourceCard->getInfo()->getRelatedCards();
    relatedCards.append(sourceCard->getInfo()->getReverseRelatedCards2Me());
    if (relatedCards.empty()) {
        return;
    }

    CardRelation *cardRelation = nullptr;
    int tokensTypesCreated = 0;

    if (relatedCards.length() == 1) {
        cardRelation = relatedCards.at(0);
        if (createRelatedFromRelation(sourceCard, cardRelation)) {
            ++tokensTypesCreated;
        }
    } else {
        QList<CardRelation *> nonExcludedRelatedCards;
        QString dbName;
        for (CardRelation *cardRelationTemp : relatedCards) {
            if (!cardRelationTemp->getIsCreateAllExclusion() && !cardRelationTemp->getDoesAttach()) {
                nonExcludedRelatedCards.append(cardRelationTemp);
            }
        }
        switch (nonExcludedRelatedCards.length()) {
            case 1: // if nonExcludedRelatedCards == 1
                cardRelation = nonExcludedRelatedCards.at(0);
                if (createRelatedFromRelation(sourceCard, cardRelation)) {
                    ++tokensTypesCreated;
                }
                break;
            // If all are marked "Exclude", then treat the situation as if none of them are.
            // We won't accept "garbage in, garbage out", here.
            case 0: // else if nonExcludedRelatedCards == 0
                for (CardRelation *cardRelationAll : relatedCards) {
                    if (!cardRelationAll->getDoesAttach() && !cardRelationAll->getIsVariable()) {
                        dbName = cardRelationAll->getName();
                        for (int i = 0; i < cardRelationAll->getDefaultCount(); ++i) {
                            createCard(sourceCard, dbName);
                        }
                        ++tokensTypesCreated;
                        if (tokensTypesCreated == 1) {
                            cardRelation = cardRelationAll;
                        }
                    }
                }
                break;
            default: // else
                for (CardRelation *cardRelationNotExcluded : nonExcludedRelatedCards) {
                    if (!cardRelationNotExcluded->getDoesAttach() && !cardRelationNotExcluded->getIsVariable()) {
                        dbName = cardRelationNotExcluded->getName();
                        for (int i = 0; i < cardRelationNotExcluded->getDefaultCount(); ++i) {
                            createCard(sourceCard, dbName);
                        }
                        ++tokensTypesCreated;
                        if (tokensTypesCreated == 1) {
                            cardRelation = cardRelationNotExcluded;
                        }
                    }
                }
                break;
        }
    }

    /*
     * If we made at least one token via "Create All Tokens"
     * then assign the first to the "Create another" shortcut.
     */
    if (cardRelation != nullptr && cardRelation->getCanCreateAnother()) {
        CardInfoPtr cardInfo = db->getCard(cardRelation->getName());
        setLastToken(cardInfo);
    }
}

bool Player::createRelatedFromRelation(const CardItem *sourceCard, const CardRelation *cardRelation)
{
    if (sourceCard == nullptr || cardRelation == nullptr) {
        return false;
    }
    QString dbName = cardRelation->getName();
    if (cardRelation->getIsVariable()) {
        bool ok;
        dialogSemaphore = true;
        int count = QInputDialog::getInt(nullptr, tr("Create tokens"), tr("Number:"), cardRelation->getDefaultCount(),
                                         1, MAX_TOKENS_PER_DIALOG, 1, &ok);
        dialogSemaphore = false;
        if (!ok) {
            return false;
        }
        for (int i = 0; i < count; ++i) {
            createCard(sourceCard, dbName);
        }
    } else if (cardRelation->getDefaultCount() > 1) {
        for (int i = 0; i < cardRelation->getDefaultCount(); ++i) {
            createCard(sourceCard, dbName);
        }
    } else {
        if (cardRelation->getDoesAttach()) {
            createAttachedCard(sourceCard, dbName);
        } else {
            createCard(sourceCard, dbName);
        }
    }
    return true;
}

void Player::createCard(const CardItem *sourceCard, const QString &dbCardName, bool attach)
{
    CardInfoPtr cardInfo = db->getCard(dbCardName);

    if (cardInfo == nullptr || sourceCard == nullptr) {
        return;
    }

    // get the target token's location
    // TODO: Define this QPoint into its own function along with the one below
    QPoint gridPoint = QPoint(-1, TableZone::clampValidTableRow(2 - cardInfo->getTableRow()));

    // create the token for the related card
    Command_CreateToken cmd;
    cmd.set_zone("table");
    cmd.set_card_name(cardInfo->getName().toStdString());
    switch (cardInfo->getColors().size()) {
        case 0:
            cmd.set_color("");
            break;
        case 1:
            cmd.set_color("m");
            break;
        default:
            cmd.set_color(cardInfo->getColors().left(1).toLower().toStdString());
            break;
    }

    cmd.set_pt(cardInfo->getPowTough().toStdString());
    if (settingsCache->getAnnotateTokens()) {
        cmd.set_annotation(cardInfo->getText().toStdString());
    } else {
        cmd.set_annotation("");
    }
    cmd.set_destroy_on_zone_change(true);
    cmd.set_target_zone(sourceCard->getZone()->getName().toStdString());
    cmd.set_x(gridPoint.x());
    cmd.set_y(gridPoint.y());

    if (attach) {
        cmd.set_target_card_id(sourceCard->getId());
    }

    sendGameCommand(cmd);
}

void Player::createAttachedCard(const CardItem *sourceCard, const QString &dbCardName)
{
    createCard(sourceCard, dbCardName, true);
}

void Player::actSayMessage()
{
    auto *a = qobject_cast<QAction *>(sender());
    Command_GameSay cmd;
    cmd.set_message(a->text().toStdString());
    sendGameCommand(cmd);
}

void Player::setCardAttrHelper(const GameEventContext &context,
                               CardItem *card,
                               CardAttribute attribute,
                               const QString &avalue,
                               bool allCards)
{
    if (card == nullptr) {
        return;
    }

    bool moveCardContext = context.HasExtension(Context_MoveCard::ext);
    switch (attribute) {
        case AttrTapped: {
            bool tapped = avalue == "1";
            if (!(!tapped && card->getDoesntUntap() && allCards)) {
                if (!allCards) {
                    emit logSetTapped(this, card, tapped);
                }
                card->setTapped(tapped, !moveCardContext);
            }
            break;
        }
        case AttrAttacking: {
            card->setAttacking(avalue == "1");
            break;
        }
        case AttrFaceDown: {
            card->setFaceDown(avalue == "1");
            break;
        }
        case AttrColor: {
            card->setColor(avalue);
            break;
        }
        case AttrAnnotation: {
            emit logSetAnnotation(this, card, avalue);
            card->setAnnotation(avalue);
            break;
        }
        case AttrDoesntUntap: {
            bool value = (avalue == "1");
            emit logSetDoesntUntap(this, card, value);
            card->setDoesntUntap(value);
            break;
        }
        case AttrPT: {
            emit logSetPT(this, card, avalue);
            card->setPT(avalue);
            break;
        }
    }
}

void Player::eventGameSay(const Event_GameSay &event)
{
    emit logSay(this, QString::fromStdString(event.message()));
}

void Player::eventShuffle(const Event_Shuffle &event)
{
    CardZone *zone = zones.value(QString::fromStdString(event.zone_name()));
    if (!zone) {
        return;
    }
    if (zone->getView() && zone->getView()->getRevealZone()) {
        zone->getView()->setWriteableRevealZone(false);
    }
    emit logShuffle(this, zone, event.start(), event.end());
}

void Player::eventRollDie(const Event_RollDie &event)
{
    emit logRollDie(this, event.sides(), event.value());
}

void Player::eventCreateArrow(const Event_CreateArrow &event)
{
    ArrowItem *arrow = addArrow(event.arrow_info());
    if (!arrow) {
        return;
    }

    auto *startCard = static_cast<CardItem *>(arrow->getStartItem());
    auto *targetCard = qgraphicsitem_cast<CardItem *>(arrow->getTargetItem());
    if (targetCard) {
        emit logCreateArrow(this, startCard->getOwner(), startCard->getName(), targetCard->getOwner(),
                            targetCard->getName(), false);
    } else {
        emit logCreateArrow(this, startCard->getOwner(), startCard->getName(), arrow->getTargetItem()->getOwner(),
                            QString(), true);
    }
}

void Player::eventDeleteArrow(const Event_DeleteArrow &event)
{
    delArrow(event.arrow_id());
}

void Player::eventCreateToken(const Event_CreateToken &event)
{
    CardZone *zone = zones.value(QString::fromStdString(event.zone_name()), 0);
    if (!zone) {
        return;
    }

    CardItem *card = new CardItem(this, QString::fromStdString(event.card_name()), event.card_id());
    // use db PT if not provided in event
    if (!QString::fromStdString(event.pt()).isEmpty()) {
        card->setPT(QString::fromStdString(event.pt()));
    } else {
        CardInfoPtr dbCard = db->getCard(QString::fromStdString(event.card_name()));
        if (dbCard) {
            card->setPT(dbCard->getPowTough());
        }
    }
    card->setColor(QString::fromStdString(event.color()));
    card->setAnnotation(QString::fromStdString(event.annotation()));
    card->setDestroyOnZoneChange(event.destroy_on_zone_change());

    emit logCreateToken(this, card->getName(), card->getPT());
    zone->addCard(card, true, event.x(), event.y());
}

void Player::eventSetCardAttr(const Event_SetCardAttr &event, const GameEventContext &context)
{
    CardZone *zone = zones.value(QString::fromStdString(event.zone_name()), 0);
    if (!zone) {
        return;
    }

    if (!event.has_card_id()) {
        const CardList &cards = zone->getCards();
        for (int i = 0; i < cards.size(); ++i) {
            setCardAttrHelper(context, cards.at(i), event.attribute(), QString::fromStdString(event.attr_value()),
                              true);
        }
        if (event.attribute() == AttrTapped) {
            emit logSetTapped(this, nullptr, event.attr_value() == "1");
        }
    } else {
        CardItem *card = zone->getCard(event.card_id(), QString());
        if (!card) {
            qDebug() << "Player::eventSetCardAttr: card id=" << event.card_id() << "not found";
            return;
        }
        setCardAttrHelper(context, card, event.attribute(), QString::fromStdString(event.attr_value()), false);
    }
}

void Player::eventSetCardCounter(const Event_SetCardCounter &event)
{
    CardZone *zone = zones.value(QString::fromStdString(event.zone_name()), 0);
    if (!zone) {
        return;
    }

    CardItem *card = zone->getCard(event.card_id(), QString());
    if (!card) {
        return;
    }

    int oldValue = card->getCounters().value(event.counter_id(), 0);
    card->setCounter(event.counter_id(), event.counter_value());
    updateCardMenu(card);
    emit logSetCardCounter(this, card->getName(), event.counter_id(), event.counter_value(), oldValue);
}

void Player::eventCreateCounter(const Event_CreateCounter &event)
{
    addCounter(event.counter_info());
}

void Player::eventSetCounter(const Event_SetCounter &event)
{
    AbstractCounter *ctr = counters.value(event.counter_id(), 0);
    if (!ctr)
        return;
    int oldValue = ctr->getValue();
    ctr->setValue(event.value());
    emit logSetCounter(this, ctr->getName(), event.value(), oldValue);
}

void Player::eventDelCounter(const Event_DelCounter &event)
{
    delCounter(event.counter_id());
}

void Player::eventDumpZone(const Event_DumpZone &event)
{
    Player *zoneOwner = game->getPlayers().value(event.zone_owner_id(), 0);
    if (!zoneOwner) {
        return;
    }
    CardZone *zone = zoneOwner->getZones().value(QString::fromStdString(event.zone_name()), 0);
    if (!zone) {
        return;
    }
    emit logDumpZone(this, zone, event.number_cards());
}

void Player::eventStopDumpZone(const Event_StopDumpZone &event)
{
    Player *zoneOwner = game->getPlayers().value(event.zone_owner_id(), 0);
    if (!zoneOwner) {
        return;
    }
    CardZone *zone = zoneOwner->getZones().value(QString::fromStdString(event.zone_name()), 0);
    if (!zone) {
        return;
    }
    emit logStopDumpZone(this, zone);
}

void Player::eventMoveCard(const Event_MoveCard &event, const GameEventContext &context)
{
    Player *startPlayer = game->getPlayers().value(event.start_player_id());
    if (!startPlayer) {
        return;
    }
    CardZone *startZone = startPlayer->getZones().value(QString::fromStdString(event.start_zone()), 0);
    Player *targetPlayer = game->getPlayers().value(event.target_player_id());
    if (!targetPlayer) {
        return;
    }
    CardZone *targetZone;
    if (event.has_target_zone()) {
        targetZone = targetPlayer->getZones().value(QString::fromStdString(event.target_zone()), 0);
    } else {
        targetZone = startZone;
    }
    if (!startZone || !targetZone) {
        return;
    }

    int position = event.position();
    int x = event.x();
    int y = event.y();

    int logPosition = position;
    int logX = x;
    if (x == -1) {
        x = 0;
    }
    CardItem *card = startZone->takeCard(position, event.card_id(), startZone != targetZone);
    if (!card) {
        return;
    }
    if (startZone != targetZone) {
        card->deleteCardInfoPopup();
    }
    if (event.has_card_name()) {
        card->setName(QString::fromStdString(event.card_name()));
    }

    if (card->getAttachedTo() && (startZone != targetZone)) {
        CardItem *parentCard = card->getAttachedTo();
        card->setAttachedTo(nullptr);
        parentCard->getZone()->reorganizeCards();
    }

    card->deleteDragItem();

    card->setId(event.new_card_id());
    card->setFaceDown(event.face_down());
    if (startZone != targetZone) {
        card->setBeingPointedAt(false);
        card->setHovered(false);

        const QList<CardItem *> &attachedCards = card->getAttachedCards();
        for (auto attachedCard : attachedCards) {
            attachedCard->setParentItem(targetZone);
        }

        if (startZone->getPlayer() != targetZone->getPlayer()) {
            card->setOwner(targetZone->getPlayer());
        }
    }

    // The log event has to be sent before the card is added to the target zone
    // because the addCard function can modify the card object.
    if (context.HasExtension(Context_UndoDraw::ext)) {
        emit logUndoDraw(this, card->getName());
    } else {
        emit logMoveCard(this, card, startZone, logPosition, targetZone, logX);
    }

    targetZone->addCard(card, true, x, y);

    // Look at all arrows from and to the card.
    // If the card was moved to another zone, delete the arrows, otherwise update them.
    QMapIterator<int, Player *> playerIterator(game->getPlayers());
    while (playerIterator.hasNext()) {
        Player *p = playerIterator.next().value();

        QList<ArrowItem *> arrowsToDelete;
        QMapIterator<int, ArrowItem *> arrowIterator(p->getArrows());
        while (arrowIterator.hasNext()) {
            ArrowItem *arrow = arrowIterator.next().value();
            if ((arrow->getStartItem() == card) || (arrow->getTargetItem() == card)) {
                if (startZone == targetZone) {
                    arrow->updatePath();
                } else {
                    arrowsToDelete.append(arrow);
                }
            }
        }
        for (auto &i : arrowsToDelete) {
            i->delArrow();
        }
    }
}

void Player::eventFlipCard(const Event_FlipCard &event)
{
    CardZone *zone = zones.value(QString::fromStdString(event.zone_name()), 0);
    if (!zone) {
        return;
    }
    CardItem *card = zone->getCard(event.card_id(), QString::fromStdString(event.card_name()));
    if (!card) {
        return;
    }
    emit logFlipCard(this, card->getName(), event.face_down());
    card->setFaceDown(event.face_down());
}

void Player::eventDestroyCard(const Event_DestroyCard &event)
{
    CardZone *zone = zones.value(QString::fromStdString(event.zone_name()), 0);
    if (!zone) {
        return;
    }

    CardItem *card = zone->getCard(event.card_id(), QString());
    if (!card) {
        return;
    }

    QList<CardItem *> attachedCards = card->getAttachedCards();
    // This list is always empty except for buggy server implementations.
    for (auto &attachedCard : attachedCards) {
        attachedCard->setAttachedTo(0);
    }

    emit logDestroyCard(this, card->getName());
    zone->takeCard(-1, event.card_id(), true);
    card->deleteLater();
}

void Player::eventAttachCard(const Event_AttachCard &event)
{
    const QMap<int, Player *> &playerList = game->getPlayers();
    Player *targetPlayer = nullptr;
    CardZone *targetZone = nullptr;
    CardItem *targetCard = nullptr;
    if (event.has_target_player_id()) {
        targetPlayer = playerList.value(event.target_player_id(), 0);
        if (targetPlayer) {
            targetZone = targetPlayer->getZones().value(QString::fromStdString(event.target_zone()), 0);
            if (targetZone) {
                targetCard = targetZone->getCard(event.target_card_id(), QString());
            }
        }
    }

    CardZone *startZone = getZones().value(QString::fromStdString(event.start_zone()), 0);
    if (!startZone) {
        return;
    }

    CardItem *startCard = startZone->getCard(event.card_id(), QString());
    if (!startCard) {
        return;
    }

    CardItem *oldParent = startCard->getAttachedTo();

    startCard->setAttachedTo(targetCard);

    startZone->reorganizeCards();
    if ((startZone != targetZone) && targetZone) {
        targetZone->reorganizeCards();
    }
    if (oldParent) {
        oldParent->getZone()->reorganizeCards();
    }

    if (targetCard) {
        emit logAttachCard(this, startCard->getName(), targetPlayer, targetCard->getName());
    } else {
        emit logUnattachCard(this, startCard->getName());
    }
}

void Player::eventDrawCards(const Event_DrawCards &event)
{
    CardZone *deck = zones.value("deck");
    CardZone *hand = zones.value("hand");

    const int listSize = event.cards_size();
    if (listSize) {
        for (int i = 0; i < listSize; ++i) {
            const ServerInfo_Card &cardInfo = event.cards(i);
            CardItem *card = deck->takeCard(0, cardInfo.id());
            card->setName(QString::fromStdString(cardInfo.name()));
            hand->addCard(card, false, -1);
        }
    } else {
        const int number = event.number();
        for (int i = 0; i < number; ++i) {
            hand->addCard(deck->takeCard(0, -1), false, -1);
        }
    }

    hand->reorganizeCards();
    deck->reorganizeCards();
    emit logDrawCards(this, event.number());
}

void Player::eventRevealCards(const Event_RevealCards &event)
{
    CardZone *zone = zones.value(QString::fromStdString(event.zone_name()));
    if (!zone) {
        return;
    }
    Player *otherPlayer = nullptr;
    if (event.has_other_player_id()) {
        otherPlayer = game->getPlayers().value(event.other_player_id());
        if (!otherPlayer) {
            return;
        }
    }

    bool peeking = false;
    QList<const ServerInfo_Card *> cardList;
    const int cardListSize = event.cards_size();
    for (int i = 0; i < cardListSize; ++i) {
        const ServerInfo_Card *temp = &event.cards(i);
        if (temp->face_down()) {
            peeking = true;
        }
        cardList.append(temp);
    }

    if (peeking) {
        for (auto &card : cardList) {
            QString cardName = QString::fromStdString(card->name());
            CardItem *cardItem = zone->getCard(card->id(), QString());
            if (!cardItem) {
                continue;
            }
            cardItem->setName(cardName);
            emit logRevealCards(this, zone, card->id(), cardName, this, true, 1);
        }
    } else {
        bool showZoneView = true;
        QString cardName;
        if (cardList.size() == 1) {
            cardName = QString::fromStdString(cardList.first()->name());
            if ((event.card_id() == 0) && dynamic_cast<PileZone *>(zone)) {
                zone->getCards().first()->setName(cardName);
                zone->update();
                showZoneView = false;
            }
        }
        if (showZoneView && !cardList.isEmpty()) {
            static_cast<GameScene *>(scene())->addRevealedZoneView(this, zone, cardList, event.grant_write_access());
        }

        emit logRevealCards(this, zone, event.card_id(), cardName, otherPlayer, false,
                            event.has_number_of_cards() ? event.number_of_cards() : cardList.size());
    }
}

void Player::eventChangeZoneProperties(const Event_ChangeZoneProperties &event)
{
    CardZone *zone = zones.value(QString::fromStdString(event.zone_name()));
    if (!zone) {
        return;
    }

    if (event.has_always_reveal_top_card()) {
        zone->setAlwaysRevealTopCard(event.always_reveal_top_card());
        emit logAlwaysRevealTopCard(this, zone, event.always_reveal_top_card());
    }
}

void Player::processGameEvent(GameEvent::GameEventType type, const GameEvent &event, const GameEventContext &context)
{
    switch (type) {
        case GameEvent::GAME_SAY:
            eventGameSay(event.GetExtension(Event_GameSay::ext));
            break;
        case GameEvent::SHUFFLE:
            eventShuffle(event.GetExtension(Event_Shuffle::ext));
            break;
        case GameEvent::ROLL_DIE:
            eventRollDie(event.GetExtension(Event_RollDie::ext));
            break;
        case GameEvent::CREATE_ARROW:
            eventCreateArrow(event.GetExtension(Event_CreateArrow::ext));
            break;
        case GameEvent::DELETE_ARROW:
            eventDeleteArrow(event.GetExtension(Event_DeleteArrow::ext));
            break;
        case GameEvent::CREATE_TOKEN:
            eventCreateToken(event.GetExtension(Event_CreateToken::ext));
            break;
        case GameEvent::SET_CARD_ATTR:
            eventSetCardAttr(event.GetExtension(Event_SetCardAttr::ext), context);
            break;
        case GameEvent::SET_CARD_COUNTER:
            eventSetCardCounter(event.GetExtension(Event_SetCardCounter::ext));
            break;
        case GameEvent::CREATE_COUNTER:
            eventCreateCounter(event.GetExtension(Event_CreateCounter::ext));
            break;
        case GameEvent::SET_COUNTER:
            eventSetCounter(event.GetExtension(Event_SetCounter::ext));
            break;
        case GameEvent::DEL_COUNTER:
            eventDelCounter(event.GetExtension(Event_DelCounter::ext));
            break;
        case GameEvent::DUMP_ZONE:
            eventDumpZone(event.GetExtension(Event_DumpZone::ext));
            break;
        case GameEvent::STOP_DUMP_ZONE:
            eventStopDumpZone(event.GetExtension(Event_StopDumpZone::ext));
            break;
        case GameEvent::MOVE_CARD:
            eventMoveCard(event.GetExtension(Event_MoveCard::ext), context);
            break;
        case GameEvent::FLIP_CARD:
            eventFlipCard(event.GetExtension(Event_FlipCard::ext));
            break;
        case GameEvent::DESTROY_CARD:
            eventDestroyCard(event.GetExtension(Event_DestroyCard::ext));
            break;
        case GameEvent::ATTACH_CARD:
            eventAttachCard(event.GetExtension(Event_AttachCard::ext));
            break;
        case GameEvent::DRAW_CARDS:
            eventDrawCards(event.GetExtension(Event_DrawCards::ext));
            break;
        case GameEvent::REVEAL_CARDS:
            eventRevealCards(event.GetExtension(Event_RevealCards::ext));
            break;
        case GameEvent::CHANGE_ZONE_PROPERTIES:
            eventChangeZoneProperties(event.GetExtension(Event_ChangeZoneProperties::ext));
            break;
        default: {
            qDebug() << "unhandled game event" << type;
        }
    }
}

void Player::setActive(bool _active)
{
    active = _active;
    table->setActive(active);
    update();
}

QRectF Player::boundingRect() const
{
    return bRect;
}

void Player::paint(QPainter * /*painter*/, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
}

void Player::processPlayerInfo(const ServerInfo_Player &info)
{
    clearCounters();
    clearArrows();

    QMapIterator<QString, CardZone *> zoneIt(zones);
    while (zoneIt.hasNext()) {
        zoneIt.next().value()->clearContents();
    }

    const int zoneListSize = info.zone_list_size();
    for (int i = 0; i < zoneListSize; ++i) {
        const ServerInfo_Zone &zoneInfo = info.zone_list(i);
        CardZone *zone = zones.value(QString::fromStdString(zoneInfo.name()), 0);
        if (!zone) {
            continue;
        }

        const int cardListSize = zoneInfo.card_list_size();
        if (!cardListSize) {
            for (int j = 0; j < zoneInfo.card_count(); ++j) {
                zone->addCard(new CardItem(this), false, -1);
            }
        } else {
            for (int j = 0; j < cardListSize; ++j) {
                const ServerInfo_Card &cardInfo = zoneInfo.card_list(j);
                CardItem *card = new CardItem(this);
                card->processCardInfo(cardInfo);
                zone->addCard(card, false, cardInfo.x(), cardInfo.y());
            }
        }
        if (zoneInfo.has_always_reveal_top_card()) {
            zone->setAlwaysRevealTopCard(zoneInfo.always_reveal_top_card());
        }

        zone->reorganizeCards();
    }

    const int counterListSize = info.counter_list_size();
    for (int i = 0; i < counterListSize; ++i) {
        addCounter(info.counter_list(i));
    }

    setConceded(info.properties().conceded());
}

void Player::processCardAttachment(const ServerInfo_Player &info)
{
    const int zoneListSize = info.zone_list_size();
    for (int i = 0; i < zoneListSize; ++i) {
        const ServerInfo_Zone &zoneInfo = info.zone_list(i);
        CardZone *zone = zones.value(QString::fromStdString(zoneInfo.name()), 0);
        if (!zone) {
            continue;
        }

        const int cardListSize = zoneInfo.card_list_size();
        for (int j = 0; j < cardListSize; ++j) {
            const ServerInfo_Card &cardInfo = zoneInfo.card_list(j);
            if (cardInfo.has_attach_player_id()) {
                CardItem *startCard = zone->getCard(cardInfo.id(), QString());
                CardItem *targetCard =
                    game->getCard(cardInfo.attach_player_id(), QString::fromStdString(cardInfo.attach_zone()),
                                  cardInfo.attach_card_id());
                if (!targetCard) {
                    continue;
                }

                startCard->setAttachedTo(targetCard);
            }
        }
    }

    const int arrowListSize = info.arrow_list_size();
    for (int i = 0; i < arrowListSize; ++i) {
        addArrow(info.arrow_list(i));
    }
}

void Player::playCard(CardItem *card, bool faceDown, bool tapped)
{
    if (card == nullptr) {
        return;
    }

    Command_MoveCard cmd;
    cmd.set_start_player_id(card->getZone()->getPlayer()->getId());
    cmd.set_start_zone(card->getZone()->getName().toStdString());
    cmd.set_target_player_id(getId());
    CardToMove *cardToMove = cmd.mutable_cards_to_move()->add_card();
    cardToMove->set_card_id(card->getId());

    CardInfoPtr info = card->getInfo();
    if (!info) {
        return;
    }

    int tableRow = info->getTableRow();
    bool playToStack = settingsCache->getPlayToStack();
    QString currentZone = card->getZone()->getName();
    if (currentZone == "stack" && tableRow == 3) {
        cmd.set_target_zone("grave");
        cmd.set_x(0);
        cmd.set_y(0);
    } else if (!faceDown &&
               ((!playToStack && tableRow == 3) || ((playToStack && tableRow != 0) && currentZone != "stack"))) {
        cmd.set_target_zone("stack");
        cmd.set_x(0);
        cmd.set_y(0);
    } else {
        tableRow = faceDown ? 2 : info->getTableRow();
        QPoint gridPoint = QPoint(-1, TableZone::clampValidTableRow(2 - tableRow));
        cardToMove->set_face_down(faceDown);
        cardToMove->set_pt(info->getPowTough().toStdString());
        cardToMove->set_tapped(faceDown ? false : tapped);
        if (tableRow != 3)
            cmd.set_target_zone("table");
        cmd.set_x(gridPoint.x());
        cmd.set_y(gridPoint.y());
    }
    sendGameCommand(cmd);
}

void Player::addCard(CardItem *card)
{
    emit newCardAdded(card);
}

void Player::deleteCard(CardItem *card)
{
    if (card == nullptr) {
        return;
    } else if (dialogSemaphore) {
        cardsToDelete.append(card);
    } else {
        card->deleteLater();
    }
}

void Player::addZone(CardZone *zone)
{
    zones.insert(zone->getName(), zone);
}

AbstractCounter *Player::addCounter(const ServerInfo_Counter &counter)
{
    return addCounter(counter.id(), QString::fromStdString(counter.name()),
                      convertColorToQColor(counter.counter_color()), counter.radius(), counter.count());
}

AbstractCounter *Player::addCounter(int counterId, const QString &name, QColor color, int radius, int value)
{
    qDebug() << "addCounter:" << getName() << counterId << name;
    if (counters.contains(counterId)) {
        return nullptr;
    }

    AbstractCounter *ctr;
    if (name == "life") {
        ctr = playerTarget->addCounter(counterId, name, value);
    } else {
        ctr = new GeneralCounter(this, counterId, name, color, radius, value, true, this);
    }
    counters.insert(counterId, ctr);
    if (countersMenu) {
        countersMenu->addMenu(ctr->getMenu());
    }
    if (shortcutsActive) {
        ctr->setShortcutsActive();
    }
    rearrangeCounters();
    return ctr;
}

void Player::delCounter(int counterId)
{
    AbstractCounter *ctr = counters.value(counterId, 0);
    if (!ctr) {
        return;
    }

    ctr->delCounter();
    counters.remove(counterId);
    rearrangeCounters();
}

void Player::clearCounters()
{
    QMapIterator<int, AbstractCounter *> counterIterator(counters);
    while (counterIterator.hasNext()) {
        counterIterator.next().value()->delCounter();
    }
    counters.clear();
}

ArrowItem *Player::addArrow(const ServerInfo_Arrow &arrow)
{
    const QMap<int, Player *> &playerList = game->getPlayers();
    Player *startPlayer = playerList.value(arrow.start_player_id(), 0);
    Player *targetPlayer = playerList.value(arrow.target_player_id(), 0);
    if (!startPlayer || !targetPlayer) {
        return nullptr;
    }

    CardZone *startZone = startPlayer->getZones().value(QString::fromStdString(arrow.start_zone()), 0);
    CardZone *targetZone = nullptr;
    if (arrow.has_target_zone()) {
        targetZone = targetPlayer->getZones().value(QString::fromStdString(arrow.target_zone()), 0);
    }
    if (!startZone || (!targetZone && arrow.has_target_zone())) {
        return nullptr;
    }

    CardItem *startCard = startZone->getCard(arrow.start_card_id(), QString());
    CardItem *targetCard = nullptr;
    if (targetZone) {
        targetCard = targetZone->getCard(arrow.target_card_id(), QString());
    }
    if (!startCard || (!targetCard && arrow.has_target_card_id())) {
        return nullptr;
    }

    if (targetCard) {
        return addArrow(arrow.id(), startCard, targetCard, convertColorToQColor(arrow.arrow_color()));
    } else {
        return addArrow(arrow.id(), startCard, targetPlayer->getPlayerTarget(),
                        convertColorToQColor(arrow.arrow_color()));
    }
}

ArrowItem *Player::addArrow(int arrowId, CardItem *startCard, ArrowTarget *targetItem, const QColor &color)
{
    auto *arrow = new ArrowItem(this, arrowId, startCard, targetItem, color);
    arrows.insert(arrowId, arrow);
    scene()->addItem(arrow);
    return arrow;
}

void Player::delArrow(int arrowId)
{
    ArrowItem *arr = arrows.value(arrowId, 0);
    if (!arr) {
        return;
    }
    arr->delArrow();
}

void Player::removeArrow(ArrowItem *arrow)
{
    if (arrow->getId() != -1) {
        arrows.remove(arrow->getId());
    }
}

void Player::clearArrows()
{
    QMapIterator<int, ArrowItem *> arrowIterator(arrows);
    while (arrowIterator.hasNext()) {
        arrowIterator.next().value()->delArrow();
    }
    arrows.clear();
}

void Player::rearrangeCounters()
{
    qreal marginTop = 80;

    // Determine total height of bounding rectangles
    qreal totalHeight = 0;
    QMapIterator<int, AbstractCounter *> counterIterator(counters);
    while (counterIterator.hasNext()) {
        counterIterator.next();
        if (counterIterator.value()->getShownInCounterArea()) {
            totalHeight += counterIterator.value()->boundingRect().height();
        }
    }

    const qreal padding = 5;
    qreal ySize = boundingRect().y() + marginTop;

    // Place objects
    for (counterIterator.toFront(); counterIterator.hasNext();) {
        AbstractCounter *ctr = counterIterator.next().value();

        if (!ctr->getShownInCounterArea()) {
            continue;
        }

        QRectF br = ctr->boundingRect();
        ctr->setPos((counterAreaWidth - br.width()) / 2, ySize);
        ySize += br.height() + padding;
    }
}

PendingCommand *Player::prepareGameCommand(const google::protobuf::Message &cmd)
{
    return game->prepareGameCommand(cmd);
}

PendingCommand *Player::prepareGameCommand(const QList<const ::google::protobuf::Message *> &cmdList)
{
    return game->prepareGameCommand(cmdList);
}

void Player::sendGameCommand(const google::protobuf::Message &command)
{
    game->sendGameCommand(command, id);
}

void Player::sendGameCommand(PendingCommand *pend)
{
    game->sendGameCommand(pend, id);
}

bool Player::clearCardsToDelete()
{
    if (cardsToDelete.isEmpty()) {
        return false;
    }

    for (auto &i : cardsToDelete) {
        if (i != nullptr) {
            i->deleteLater();
        }
    }
    cardsToDelete.clear();

    return true;
}

void Player::actMoveCardXCardsFromTop()
{
    bool ok;
    int number = QInputDialog::getInt(nullptr, tr("Place card X cards from top of library"),
                                      tr("How many cards from the top of the deck should this card be placed:"),
                                      defaultNumberTopCardsToPlaceBelow, 1, 2000000000, 1, &ok);
    number--;

    if (!ok) {
        return;
    }

    defaultNumberTopCardsToPlaceBelow = number;

    QList<QGraphicsItem *> sel = scene()->selectedItems();
    QList<CardItem *> cardList;
    while (!sel.isEmpty()) {
        cardList.append(qgraphicsitem_cast<CardItem *>(sel.takeFirst()));
    }

    QList<const ::google::protobuf::Message *> commandList;
    ListOfCardsToMove idList;
    for (auto &i : cardList) {
        idList.add_card()->set_card_id(i->getId());
    }

    if (cardList.isEmpty()) {
        return;
    }

    int startPlayerId = cardList[0]->getZone()->getPlayer()->getId();
    QString startZone = cardList[0]->getZone()->getName();

    auto *cmd = new Command_MoveCard;
    cmd->set_start_player_id(startPlayerId);
    cmd->set_start_zone(startZone.toStdString());
    cmd->mutable_cards_to_move()->CopyFrom(idList);
    cmd->set_target_player_id(getId());
    cmd->set_target_zone("deck");
    cmd->set_x(number);
    cmd->set_y(0);
    commandList.append(cmd);

    if (local) {
        sendGameCommand(prepareGameCommand(commandList));
    } else {
        game->sendGameCommand(prepareGameCommand(commandList));
    }
}

void Player::cardMenuAction()
{
    auto *a = dynamic_cast<QAction *>(sender());
    QList<QGraphicsItem *> sel = scene()->selectedItems();
    QList<CardItem *> cardList;
    while (!sel.isEmpty()) {
        cardList.append(qgraphicsitem_cast<CardItem *>(sel.takeFirst()));
    }

    QList<const ::google::protobuf::Message *> commandList;
    if (a->data().toInt() <= (int)cmClone) {
        for (auto card : cardList) {
            switch (static_cast<CardMenuActionType>(a->data().toInt())) {
                // Leaving both for compatibility with server
                case cmUntap:
                    // fallthrough
                case cmTap: {
                    auto *cmd = new Command_SetCardAttr;
                    cmd->set_zone(card->getZone()->getName().toStdString());
                    cmd->set_card_id(card->getId());
                    cmd->set_attribute(AttrTapped);
                    cmd->set_attr_value(std::to_string(1 - static_cast<int>(card->getTapped())));
                    commandList.append(cmd);
                    break;
                }
                case cmDoesntUntap: {
                    auto *cmd = new Command_SetCardAttr;
                    cmd->set_zone(card->getZone()->getName().toStdString());
                    cmd->set_card_id(card->getId());
                    cmd->set_attribute(AttrDoesntUntap);
                    cmd->set_attr_value(card->getDoesntUntap() ? "0" : "1");
                    commandList.append(cmd);
                    break;
                }
                case cmFlip: {
                    auto *cmd = new Command_FlipCard;
                    cmd->set_zone(card->getZone()->getName().toStdString());
                    cmd->set_card_id(card->getId());
                    cmd->set_face_down(!card->getFaceDown());
                    if (card->getFaceDown()) {
                        CardInfoPtr ci = card->getInfo();
                        if (ci) {
                            cmd->set_pt(ci->getPowTough().toStdString());
                        }
                    }
                    commandList.append(cmd);
                    break;
                }
                case cmPeek: {
                    auto *cmd = new Command_RevealCards;
                    cmd->set_zone_name(card->getZone()->getName().toStdString());
                    cmd->set_card_id(card->getId());
                    cmd->set_player_id(id);
                    commandList.append(cmd);
                    break;
                }
                case cmClone: {
                    auto *cmd = new Command_CreateToken;
                    cmd->set_zone("table");
                    cmd->set_card_name(card->getName().toStdString());
                    cmd->set_color(card->getColor().toStdString());
                    cmd->set_pt(card->getPT().toStdString());
                    cmd->set_annotation(card->getAnnotation().toStdString());
                    cmd->set_destroy_on_zone_change(true);
                    cmd->set_x(-1);
                    cmd->set_y(card->getGridPoint().y());
                    commandList.append(cmd);
                    break;
                }
                default:
                    break;
            }
        }
    } else {
        ListOfCardsToMove idList;
        for (auto &i : cardList) {
            idList.add_card()->set_card_id(i->getId());
        }
        int startPlayerId = cardList[0]->getZone()->getPlayer()->getId();
        QString startZone = cardList[0]->getZone()->getName();

        switch (static_cast<CardMenuActionType>(a->data().toInt())) {
            case cmMoveToTopLibrary: {
                auto *cmd = new Command_MoveCard;
                cmd->set_start_player_id(startPlayerId);
                cmd->set_start_zone(startZone.toStdString());
                cmd->mutable_cards_to_move()->CopyFrom(idList);
                cmd->set_target_player_id(getId());
                cmd->set_target_zone("deck");
                cmd->set_x(0);
                cmd->set_y(0);
                commandList.append(cmd);
                break;
            }
            case cmMoveToBottomLibrary: {
                auto *cmd = new Command_MoveCard;
                cmd->set_start_player_id(startPlayerId);
                cmd->set_start_zone(startZone.toStdString());
                cmd->mutable_cards_to_move()->CopyFrom(idList);
                cmd->set_target_player_id(getId());
                cmd->set_target_zone("deck");
                cmd->set_x(-1);
                cmd->set_y(0);

                if (idList.card_size() > 1) {
                    auto *scmd = new Command_Shuffle;
                    scmd->set_zone_name("deck");
                    scmd->set_start(-idList.card_size());
                    scmd->set_end(-1);
                    // Server process events backwards, so...
                    commandList.append(scmd);
                }

                commandList.append(cmd);
                break;
            }
            case cmMoveToHand: {
                auto *cmd = new Command_MoveCard;
                cmd->set_start_player_id(startPlayerId);
                cmd->set_start_zone(startZone.toStdString());
                cmd->mutable_cards_to_move()->CopyFrom(idList);
                cmd->set_target_player_id(getId());
                cmd->set_target_zone("hand");
                cmd->set_x(0);
                cmd->set_y(0);
                commandList.append(cmd);
                break;
            }
            case cmMoveToGraveyard: {
                auto *cmd = new Command_MoveCard;
                cmd->set_start_player_id(startPlayerId);
                cmd->set_start_zone(startZone.toStdString());
                cmd->mutable_cards_to_move()->CopyFrom(idList);
                cmd->set_target_player_id(getId());
                cmd->set_target_zone("grave");
                cmd->set_x(0);
                cmd->set_y(0);
                commandList.append(cmd);
                break;
            }
            case cmMoveToExile: {
                auto *cmd = new Command_MoveCard;
                cmd->set_start_player_id(startPlayerId);
                cmd->set_start_zone(startZone.toStdString());
                cmd->mutable_cards_to_move()->CopyFrom(idList);
                cmd->set_target_player_id(getId());
                cmd->set_target_zone("rfg");
                cmd->set_x(0);
                cmd->set_y(0);
                commandList.append(cmd);
                break;
            }
            default:
                break;
        }
    }

    if (local) {
        sendGameCommand(prepareGameCommand(commandList));
    } else {
        game->sendGameCommand(prepareGameCommand(commandList));
    }
}

void Player::actIncPT(int deltaP, int deltaT)
{
    QString ptString = "+" + QString::number(deltaP) + "/+" + QString::number(deltaT);
    int playerid = id;

    QList<const ::google::protobuf::Message *> commandList;
    QListIterator<QGraphicsItem *> j(scene()->selectedItems());
    while (j.hasNext()) {
        auto *card = static_cast<CardItem *>(j.next());
        auto *cmd = new Command_SetCardAttr;
        cmd->set_zone(card->getZone()->getName().toStdString());
        cmd->set_card_id(card->getId());
        cmd->set_attribute(AttrPT);
        cmd->set_attr_value(ptString.toStdString());
        commandList.append(cmd);

        if (local) {
            playerid = card->getZone()->getPlayer()->getId();
        }
    }

    game->sendGameCommand(prepareGameCommand(commandList), playerid);
}

void Player::actResetPT()
{
    int playerid = id;
    QList<const ::google::protobuf::Message *> commandList;
    QListIterator<QGraphicsItem *> selected(scene()->selectedItems());
    while (selected.hasNext()) {
        auto *card = static_cast<CardItem *>(selected.next());
        QString ptString;
        if (!card->getFaceDown()) { // leave the pt empty if the card is face down
            CardInfoPtr info = card->getInfo();
            if (info) {
                ptString = info->getPowTough();
            }
        }
        if (ptString == card->getPT()) {
            continue;
        }
        QString zoneName = card->getZone()->getName();
        auto *cmd = new Command_SetCardAttr;
        cmd->set_zone(zoneName.toStdString());
        cmd->set_card_id(card->getId());
        cmd->set_attribute(AttrPT);
        cmd->set_attr_value(ptString.toStdString());
        commandList.append(cmd);

        if (local) {
            playerid = card->getZone()->getPlayer()->getId();
        }
    }

    game->sendGameCommand(prepareGameCommand(commandList), playerid);
}

void Player::actSetPT()
{
    QString oldPT;
    int playerid = id;

    QListIterator<QGraphicsItem *> i(scene()->selectedItems());
    while (i.hasNext()) {
        auto *card = static_cast<CardItem *>(i.next());
        if (!card->getPT().isEmpty()) {
            oldPT = card->getPT();
        }
    }
    bool ok;
    dialogSemaphore = true;
    QString pt = QInputDialog::getText(nullptr, tr("Set power/toughness"), tr("Please enter the new PT:"),
                                       QLineEdit::Normal, oldPT, &ok);
    dialogSemaphore = false;
    if (clearCardsToDelete()) {
        return;
    }
    if (!ok) {
        return;
    }

    QList<const ::google::protobuf::Message *> commandList;
    QListIterator<QGraphicsItem *> j(scene()->selectedItems());
    while (j.hasNext()) {
        auto *card = static_cast<CardItem *>(j.next());
        auto *cmd = new Command_SetCardAttr;
        cmd->set_zone(card->getZone()->getName().toStdString());
        cmd->set_card_id(card->getId());
        cmd->set_attribute(AttrPT);
        cmd->set_attr_value(pt.toStdString());
        commandList.append(cmd);

        if (local) {
            playerid = card->getZone()->getPlayer()->getId();
        }
    }

    game->sendGameCommand(prepareGameCommand(commandList), playerid);
}

void Player::actDrawArrow()
{
    if (!game->getActiveCard()) {
        return;
    }

    game->getActiveCard()->drawArrow(Qt::red);
}

void Player::actIncP()
{
    actIncPT(1, 0);
}

void Player::actDecP()
{
    actIncPT(-1, 0);
}

void Player::actIncT()
{
    actIncPT(0, 1);
}

void Player::actDecT()
{
    actIncPT(0, -1);
}

void Player::actIncPT()
{
    actIncPT(1, 1);
}

void Player::actDecPT()
{
    actIncPT(-1, -1);
}

void Player::actSetAnnotation()
{
    QString oldAnnotation;
    QListIterator<QGraphicsItem *> i(scene()->selectedItems());
    while (i.hasNext()) {
        auto *card = static_cast<CardItem *>(i.next());
        if (!card->getAnnotation().isEmpty()) {
            oldAnnotation = card->getAnnotation();
        }
    }

    bool ok;
    dialogSemaphore = true;
    QString annotation = QInputDialog::getText(nullptr, tr("Set annotation"), tr("Please enter the new annotation:"),
                                               QLineEdit::Normal, oldAnnotation, &ok);
    dialogSemaphore = false;
    if (clearCardsToDelete()) {
        return;
    }
    if (!ok) {
        return;
    }

    QList<const ::google::protobuf::Message *> commandList;
    i.toFront();
    while (i.hasNext()) {
        auto *card = static_cast<CardItem *>(i.next());
        auto *cmd = new Command_SetCardAttr;
        cmd->set_zone(card->getZone()->getName().toStdString());
        cmd->set_card_id(card->getId());
        cmd->set_attribute(AttrAnnotation);
        cmd->set_attr_value(annotation.toStdString());
        commandList.append(cmd);
    }
    sendGameCommand(prepareGameCommand(commandList));
}

void Player::actAttach()
{
    if (!game->getActiveCard()) {
        return;
    }

    auto *arrow = new ArrowAttachItem(game->getActiveCard());
    scene()->addItem(arrow);
    arrow->grabMouse();
}

void Player::actUnattach()
{
    if (!game->getActiveCard()) {
        return;
    }

    Command_AttachCard cmd;
    cmd.set_start_zone(game->getActiveCard()->getZone()->getName().toStdString());
    cmd.set_card_id(game->getActiveCard()->getId());
    sendGameCommand(cmd);
}

void Player::actCardCounterTrigger()
{
    auto *action = static_cast<QAction *>(sender());
    int counterId = action->data().toInt() / 1000;
    QList<const ::google::protobuf::Message *> commandList;
    switch (action->data().toInt() % 1000) { // TODO: define case numbers
        case 9: {
            QListIterator<QGraphicsItem *> i(scene()->selectedItems());
            while (i.hasNext()) {
                auto *card = static_cast<CardItem *>(i.next());
                if (card->getCounters().value(counterId, 0) < MAX_COUNTERS_ON_CARD) {
                    auto *cmd = new Command_SetCardCounter;
                    cmd->set_zone(card->getZone()->getName().toStdString());
                    cmd->set_card_id(card->getId());
                    cmd->set_counter_id(counterId);
                    cmd->set_counter_value(card->getCounters().value(counterId, 0) + 1);
                    commandList.append(cmd);
                }
            }
            break;
        }
        case 10: {
            QListIterator<QGraphicsItem *> i(scene()->selectedItems());
            while (i.hasNext()) {
                auto *card = static_cast<CardItem *>(i.next());
                if (card->getCounters().value(counterId, 0)) {
                    auto *cmd = new Command_SetCardCounter;
                    cmd->set_zone(card->getZone()->getName().toStdString());
                    cmd->set_card_id(card->getId());
                    cmd->set_counter_id(counterId);
                    cmd->set_counter_value(card->getCounters().value(counterId, 0) - 1);
                    commandList.append(cmd);
                }
            }
            break;
        }
        case 11: {
            bool ok;
            dialogSemaphore = true;
            int number =
                QInputDialog::getInt(nullptr, tr("Set counters"), tr("Number:"), 0, 0, MAX_COUNTERS_ON_CARD, 1, &ok);
            dialogSemaphore = false;
            if (clearCardsToDelete() || !ok) {
                return;
            }

            QListIterator<QGraphicsItem *> i(scene()->selectedItems());
            while (i.hasNext()) {
                auto *card = static_cast<CardItem *>(i.next());
                auto *cmd = new Command_SetCardCounter;
                cmd->set_zone(card->getZone()->getName().toStdString());
                cmd->set_card_id(card->getId());
                cmd->set_counter_id(counterId);
                cmd->set_counter_value(number);
                commandList.append(cmd);
            }
            break;
        }
        default:;
    }
    sendGameCommand(prepareGameCommand(commandList));
}

void Player::actPlay()
{
    if (!game->getActiveCard()) {
        return;
    }

    bool cipt = game->getActiveCard()->getInfo() ? game->getActiveCard()->getInfo()->getCipt() : false;
    playCard(game->getActiveCard(), false, cipt);
}

void Player::actHide()
{
    if (!game->getActiveCard()) {
        return;
    }

    game->getActiveCard()->getZone()->removeCard(game->getActiveCard());
}

void Player::actPlayFacedown()
{
    if (!game->getActiveCard()) {
        return;
    }

    playCard(game->getActiveCard(), true, false);
}

void Player::refreshShortcuts()
{
    if (shortcutsActive) {
        setShortcutsActive();

        for (const CardItem *cardItem : table->getCards()) {
            updateCardMenu(cardItem);
        }
    }
}

void Player::updateCardMenu(const CardItem *card)
{
    // If bad card OR is a spectator (as spectators don't need card menus), return
    if (card == nullptr || game->isSpectator()) {
        return;
    }

    QMenu *cardMenu = card->getCardMenu();
    QMenu *ptMenu = card->getPTMenu();
    QMenu *moveMenu = card->getMoveMenu();

    cardMenu->clear();

    bool revealedCard = false;
    bool writeableCard = getLocal();
    if (card->getZone() && card->getZone()->getIsView()) {
        auto *view = dynamic_cast<ZoneViewZone *>(card->getZone());
        if (view->getRevealZone()) {
            if (view->getWriteableRevealZone()) {
                writeableCard = true;
            } else {
                revealedCard = true;
            }
        }
    }

    if (revealedCard) {
        cardMenu->addAction(aHide);
        addRelatedCardView(card, cardMenu);
    } else if (writeableCard) {
        if (moveMenu->isEmpty()) {
            moveMenu->addAction(aMoveToTopLibrary);
            moveMenu->addAction(aMoveToXfromTopOfLibrary);
            moveMenu->addAction(aMoveToBottomLibrary);
            moveMenu->addSeparator();
            moveMenu->addAction(aMoveToHand);
            moveMenu->addSeparator();
            moveMenu->addAction(aMoveToGraveyard);
            moveMenu->addSeparator();
            moveMenu->addAction(aMoveToExile);
        }

        if (card->getZone()) {
            if (card->getZone()->getName() == "table") {
                // Card is on the battlefield

                if (ptMenu->isEmpty()) {
                    ptMenu->addAction(aIncP);
                    ptMenu->addAction(aDecP);
                    ptMenu->addSeparator();
                    ptMenu->addAction(aIncT);
                    ptMenu->addAction(aDecT);
                    ptMenu->addSeparator();
                    ptMenu->addAction(aIncPT);
                    ptMenu->addAction(aDecPT);
                    ptMenu->addSeparator();
                    ptMenu->addAction(aSetPT);
                    ptMenu->addAction(aResetPT);
                }

                cardMenu->addAction(aTap);
                cardMenu->addAction(aDoesntUntap);
                cardMenu->addAction(aFlip);
                if (card->getFaceDown()) {
                    cardMenu->addAction(aPeek);
                }

                addRelatedCardView(card, cardMenu);
                addRelatedCardActions(card, cardMenu);

                cardMenu->addSeparator();
                cardMenu->addAction(aAttach);
                if (card->getAttachedTo()) {
                    cardMenu->addAction(aUnattach);
                }
                cardMenu->addAction(aDrawArrow);
                cardMenu->addSeparator();
                cardMenu->addMenu(ptMenu);
                cardMenu->addAction(aSetAnnotation);
                cardMenu->addSeparator();
                cardMenu->addAction(aClone);
                cardMenu->addMenu(moveMenu);

                for (int i = 0; i < aAddCounter.size(); ++i) {
                    cardMenu->addSeparator();
                    cardMenu->addAction(aAddCounter[i]);
                    if (card->getCounters().contains(i)) {
                        cardMenu->addAction(aRemoveCounter[i]);
                    }
                    cardMenu->addAction(aSetCounter[i]);
                }
                cardMenu->addSeparator();
            } else if (card->getZone()->getName() == "stack") {
                // Card is on the stack
                cardMenu->addAction(aDrawArrow);
                cardMenu->addSeparator();
                cardMenu->addAction(aClone);
                cardMenu->addMenu(moveMenu);

                addRelatedCardView(card, cardMenu);
                addRelatedCardActions(card, cardMenu);
            } else if (card->getZone()->getName() == "rfg" || card->getZone()->getName() == "grave") {
                // Card is in the graveyard or exile
                cardMenu->addAction(aPlay);
                cardMenu->addAction(aPlayFacedown);
                cardMenu->addSeparator();
                cardMenu->addAction(aClone);
                cardMenu->addMenu(moveMenu);

                addRelatedCardView(card, cardMenu);
                addRelatedCardActions(card, cardMenu);
            } else {
                // Card is in hand or a custom zone specified by server
                cardMenu->addAction(aPlay);
                cardMenu->addAction(aPlayFacedown);
                cardMenu->addMenu(moveMenu);
                addRelatedCardView(card, cardMenu);
            }
        } else {
            cardMenu->addMenu(moveMenu);
        }
    } else {
        if (card->getZone() && card->getZone()->getName() != "hand") {
            cardMenu->addAction(aDrawArrow);
            cardMenu->addSeparator();
            addRelatedCardView(card, cardMenu);
            addRelatedCardActions(card, cardMenu);
            cardMenu->addSeparator();
            cardMenu->addAction(aClone);
        }
    }
}

void Player::addRelatedCardView(const CardItem *card, QMenu *cardMenu)
{
    if (card == nullptr || cardMenu == nullptr || card->getInfo() == nullptr) {
        return;
    }

    QList<CardRelation *> relatedCards = card->getInfo()->getRelatedCards();
    if (relatedCards.isEmpty()) {
        return;
    }

    cardMenu->addSeparator();
    auto viewRelatedCards = new QMenu(tr("View related cards"));
    cardMenu->addMenu(viewRelatedCards);
    auto *signalMapper = new QSignalMapper(this);
    for (const CardRelation *relatedCard : relatedCards) {
        QAction *viewCard = viewRelatedCards->addAction(relatedCard->getName());
        connect(viewCard, SIGNAL(triggered()), signalMapper, SLOT(map()));
        signalMapper->setMapping(viewCard, relatedCard->getName());
    }
    connect(signalMapper, SIGNAL(mapped(const QString &)), game, SLOT(viewCardInfo(const QString &)));
}

void Player::addRelatedCardActions(const CardItem *card, QMenu *cardMenu)
{
    if (card == nullptr || cardMenu == nullptr || card->getInfo() == nullptr) {
        return;
    }

    QList<CardRelation *> relatedCards(card->getInfo()->getRelatedCards());
    relatedCards.append(card->getInfo()->getReverseRelatedCards2Me());
    if (relatedCards.empty()) {
        return;
    }

    cardMenu->addSeparator();
    int index = 0;
    QAction *createRelatedCards = nullptr;
    for (const CardRelation *cardRelation : relatedCards) {
        CardInfoPtr relatedCard = db->getCard(cardRelation->getName());
        if (relatedCard == nullptr)
            continue;
        QString relatedCardName;
        if (relatedCard->getPowTough().size() > 0) {
            relatedCardName = relatedCard->getPowTough() + " " + relatedCard->getName(); // "n/n name"
        } else {
            relatedCardName = relatedCard->getName(); // "name"
        }

        QString text = tr("Token: ");
        if (cardRelation->getDoesAttach()) {
            text += tr("Attach to ") + "\"" + relatedCardName + "\"";
        } else if (cardRelation->getIsVariable()) {
            text += "X " + relatedCardName;
        } else if (cardRelation->getDefaultCount() != 1) {
            text += QString::number(cardRelation->getDefaultCount()) + "x " + relatedCardName;
        } else {
            text += relatedCardName;
        }

        if (createRelatedCards == nullptr) {
            if (relatedCards.length() == 1) {
                createRelatedCards = new QAction(text, this); // set actCreateAllRelatedCards with this text
                break; // do not set an individual entry as there is only one entry
            } else {
                createRelatedCards = new QAction(tr("All tokens"), this);
            }
        }

        auto *createRelated = new QAction(text, this);
        createRelated->setData(QVariant(index++));
        connect(createRelated, SIGNAL(triggered()), this, SLOT(actCreateRelatedCard()));
        cardMenu->addAction(createRelated);
    }

    if (shortcutsActive) {
        createRelatedCards->setShortcut(settingsCache->shortcuts().getSingleShortcut("Player/aCreateRelatedTokens"));
    }
    connect(createRelatedCards, SIGNAL(triggered()), this, SLOT(actCreateAllRelatedCards()));
    cardMenu->addAction(createRelatedCards);
}

void Player::setCardMenu(QMenu *menu)
{
    if (aCardMenu) {
        aCardMenu->setMenu(menu);
    }
}

QMenu *Player::getCardMenu() const
{
    if (aCardMenu) {
        return aCardMenu->menu();
    }
    return nullptr;
}

QString Player::getName() const
{
    return QString::fromStdString(userInfo->name());
}

qreal Player::getMinimumWidth() const
{
    qreal result = table->getMinimumWidth() + CARD_HEIGHT + 15 + counterAreaWidth + stack->boundingRect().width();
    if (!settingsCache->getHorizontalHand()) {
        result += hand->boundingRect().width();
    }
    return result;
}

void Player::setGameStarted()
{
    if (local) {
        aAlwaysRevealTopCard->setChecked(false);
    }
    setConceded(false);
}

void Player::setConceded(bool _conceded)
{
    conceded = _conceded;
    setVisible(!conceded);
    if (conceded) {
        clear();
    }
    emit playerCountChanged();
}

void Player::setMirrored(bool _mirrored)
{
    if (mirrored != _mirrored) {
        mirrored = _mirrored;
        rearrangeZones();
    }
}

void Player::processSceneSizeChange(int newPlayerWidth)
{
    // Extend table (and hand, if horizontal) to accommodate the new player width.
    qreal tableWidth = newPlayerWidth - CARD_HEIGHT - 15 - counterAreaWidth - stack->boundingRect().width();
    if (!settingsCache->getHorizontalHand()) {
        tableWidth -= hand->boundingRect().width();
    }

    table->setWidth(tableWidth);
    hand->setWidth(tableWidth + stack->boundingRect().width());
}

void Player::setLastToken(CardInfoPtr cardInfo)
{
    if (cardInfo == nullptr || aCreateAnotherToken == nullptr) {
        return;
    }

    lastTokenName = cardInfo->getName();
    lastTokenColor = cardInfo->getColors().isEmpty() ? QString() : cardInfo->getColors().left(1).toLower();
    lastTokenPT = cardInfo->getPowTough();
    lastTokenAnnotation = settingsCache->getAnnotateTokens() ? cardInfo->getText() : "";
    lastTokenTableRow = TableZone::clampValidTableRow(2 - cardInfo->getTableRow());
    lastTokenDestroy = true;
    aCreateAnotherToken->setText(tr("C&reate another %1 token").arg(lastTokenName));
    aCreateAnotherToken->setEnabled(true);
}
