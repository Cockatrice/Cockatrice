#include "player.h"

#include "../../client/get_text_with_max.h"
#include "../../client/tabs/tab_game.h"
#include "../../client/ui/theme_manager.h"
#include "../../deck/deck_loader.h"
#include "../../dialogs/dlg_move_top_cards_until.h"
#include "../../dialogs/dlg_roll_dice.h"
#include "../../main.h"
#include "../../settings/cache_settings.h"
#include "../../settings/card_counter_settings.h"
#include "../board/arrow_item.h"
#include "../board/card_item.h"
#include "../board/card_list.h"
#include "../board/counter_general.h"
#include "../cards/card_database.h"
#include "../cards/card_database_manager.h"
#include "../game_scene.h"
#include "../hand_counter.h"
#include "../zones/card_zone.h"
#include "../zones/hand_zone.h"
#include "../zones/pile_zone.h"
#include "../zones/stack_zone.h"
#include "../zones/table_zone.h"
#include "../zones/view_zone.h"
#include "../zones/view_zone_widget.h"
#include "color.h"
#include "pb/command_attach_card.pb.h"
#include "pb/command_change_zone_properties.pb.h"
#include "pb/command_concede.pb.h"
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
#include "pb/serverinfo_player.pb.h"
#include "pb/serverinfo_user.pb.h"
#include "pb/serverinfo_zone.pb.h"
#include "player_target.h"
#include "trice_limits.h"

#include <QDebug>
#include <QMenu>
#include <QMessageBox>
#include <QMetaType>
#include <QPainter>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QtConcurrent>

// milliseconds in between triggers of the move top cards until action
static constexpr int MOVE_TOP_CARD_UNTIL_INTERVAL = 100;

PlayerArea::PlayerArea(QGraphicsItem *parentItem) : QObject(), QGraphicsItem(parentItem)
{
    setCacheMode(DeviceCoordinateCache);
    connect(themeManager, &ThemeManager::themeChanged, this, &PlayerArea::updateBg);
    updateBg();
}

void PlayerArea::updateBg()
{
    update();
}

void PlayerArea::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    QBrush brush = themeManager->getExtraBgBrush(ThemeManager::Player, playerZoneId);
    painter->fillRect(boundingRect(), brush);
}

void PlayerArea::setSize(qreal width, qreal height)
{
    prepareGeometryChange();
    bRect = QRectF(0, 0, width, height);
}

void PlayerArea::setPlayerZoneId(int _playerZoneId)
{
    playerZoneId = _playerZoneId;
}

Player::Player(const ServerInfo_User &info, int _id, bool _local, bool _judge, TabGame *_parent)
    : QObject(_parent), game(_parent), movingCardsUntil(false), shortcutsActive(false), lastTokenTableRow(0), id(_id),
      active(false), local(_local), judge(_judge), mirrored(false), handVisible(false), conceded(false), zoneId(0),
      dialogSemaphore(false), deck(nullptr)
{
    userInfo = new ServerInfo_User;
    userInfo->CopyFrom(info);

    connect(&SettingsCache::instance(), &SettingsCache::horizontalHandChanged, this, &Player::rearrangeZones);
    connect(&SettingsCache::instance(), &SettingsCache::handJustificationChanged, this, &Player::rearrangeZones);

    playerArea = new PlayerArea(this);

    playerTarget = new PlayerTarget(this, playerArea, game);
    qreal avatarMargin = (counterAreaWidth + CARD_HEIGHT + 15 - playerTarget->boundingRect().width()) / 2.0;
    playerTarget->setPos(QPointF(avatarMargin, avatarMargin));

    auto *_deck = addZone(new PileZone(this, "deck", true, false, playerArea));
    QPointF base = QPointF(counterAreaWidth + (CARD_HEIGHT - CARD_WIDTH + 15) / 2.0,
                           10 + playerTarget->boundingRect().height() + 5 - (CARD_HEIGHT - CARD_WIDTH) / 2.0);
    _deck->setPos(base);

    qreal h = _deck->boundingRect().width() + 5;

    auto *handCounter = new HandCounter(playerArea);
    handCounter->setPos(base + QPointF(0, h + 10));
    qreal h2 = handCounter->boundingRect().height();

    PileZone *grave = addZone(new PileZone(this, "grave", false, true, playerArea));
    grave->setPos(base + QPointF(0, h + h2 + 10));

    PileZone *rfg = addZone(new PileZone(this, "rfg", false, true, playerArea));
    rfg->setPos(base + QPointF(0, 2 * h + h2 + 10));

    PileZone *sb = addZone(new PileZone(this, "sb", false, false, playerArea));
    sb->setVisible(false);

    table = addZone(new TableZone(this, "table", this));
    connect(table, &TableZone::sizeChanged, this, &Player::updateBoundingRect);

    stack = addZone(new StackZone(this, (int)table->boundingRect().height(), this));

    hand = addZone(new HandZone(this,
                                _local || _judge || (_parent->getSpectator() && _parent->getSpectatorsSeeEverything()),
                                (int)table->boundingRect().height(), this));
    connect(hand, &HandZone::cardCountChanged, handCounter, &HandCounter::updateNumber);
    connect(handCounter, &HandCounter::showContextMenu, hand, &HandZone::showContextMenu);

    updateBoundingRect();

    if (local || judge) {
        connect(_parent, &TabGame::playerAdded, this, &Player::addPlayer);
        connect(_parent, &TabGame::playerRemoved, this, &Player::removePlayer);
    }

    if (local || judge) {
        aMoveHandToTopLibrary = new QAction(this);
        aMoveHandToTopLibrary->setData(QList<QVariant>() << "deck" << 0);
        aMoveHandToBottomLibrary = new QAction(this);
        aMoveHandToBottomLibrary->setData(QList<QVariant>() << "deck" << -1);
        aMoveHandToGrave = new QAction(this);
        aMoveHandToGrave->setData(QList<QVariant>() << "grave" << 0);
        aMoveHandToRfg = new QAction(this);
        aMoveHandToRfg->setData(QList<QVariant>() << "rfg" << 0);

        connect(aMoveHandToTopLibrary, &QAction::triggered, hand, &HandZone::moveAllToZone);
        connect(aMoveHandToBottomLibrary, &QAction::triggered, hand, &HandZone::moveAllToZone);
        connect(aMoveHandToGrave, &QAction::triggered, hand, &HandZone::moveAllToZone);
        connect(aMoveHandToRfg, &QAction::triggered, hand, &HandZone::moveAllToZone);

        aMoveGraveToTopLibrary = new QAction(this);
        aMoveGraveToTopLibrary->setData(QList<QVariant>() << "deck" << 0);
        aMoveGraveToBottomLibrary = new QAction(this);
        aMoveGraveToBottomLibrary->setData(QList<QVariant>() << "deck" << -1);
        aMoveGraveToHand = new QAction(this);
        aMoveGraveToHand->setData(QList<QVariant>() << "hand" << 0);
        aMoveGraveToRfg = new QAction(this);
        aMoveGraveToRfg->setData(QList<QVariant>() << "rfg" << 0);

        connect(aMoveGraveToTopLibrary, &QAction::triggered, grave, &PileZone::moveAllToZone);
        connect(aMoveGraveToBottomLibrary, &QAction::triggered, grave, &PileZone::moveAllToZone);
        connect(aMoveGraveToHand, &QAction::triggered, grave, &PileZone::moveAllToZone);
        connect(aMoveGraveToRfg, &QAction::triggered, grave, &PileZone::moveAllToZone);

        aMoveRfgToTopLibrary = new QAction(this);
        aMoveRfgToTopLibrary->setData(QList<QVariant>() << "deck" << 0);
        aMoveRfgToBottomLibrary = new QAction(this);
        aMoveRfgToBottomLibrary->setData(QList<QVariant>() << "deck" << -1);
        aMoveRfgToHand = new QAction(this);
        aMoveRfgToHand->setData(QList<QVariant>() << "hand" << 0);
        aMoveRfgToGrave = new QAction(this);
        aMoveRfgToGrave->setData(QList<QVariant>() << "grave" << 0);

        connect(aMoveRfgToTopLibrary, &QAction::triggered, rfg, &PileZone::moveAllToZone);
        connect(aMoveRfgToBottomLibrary, &QAction::triggered, rfg, &PileZone::moveAllToZone);
        connect(aMoveRfgToHand, &QAction::triggered, rfg, &PileZone::moveAllToZone);
        connect(aMoveRfgToGrave, &QAction::triggered, rfg, &PileZone::moveAllToZone);

        aViewLibrary = new QAction(this);
        connect(aViewLibrary, &QAction::triggered, this, &Player::actViewLibrary);
        aViewHand = new QAction(this);
        connect(aViewHand, &QAction::triggered, this, &Player::actViewHand);
        aSortHand = new QAction(this);
        connect(aSortHand, &QAction::triggered, this, &Player::actSortHand);

        aViewTopCards = new QAction(this);
        connect(aViewTopCards, &QAction::triggered, this, &Player::actViewTopCards);
        aViewBottomCards = new QAction(this);
        connect(aViewBottomCards, &QAction::triggered, this, &Player::actViewBottomCards);
        aAlwaysRevealTopCard = new QAction(this);
        aAlwaysRevealTopCard->setCheckable(true);
        connect(aAlwaysRevealTopCard, &QAction::triggered, this, &Player::actAlwaysRevealTopCard);
        aAlwaysLookAtTopCard = new QAction(this);
        aAlwaysLookAtTopCard->setCheckable(true);
        connect(aAlwaysLookAtTopCard, &QAction::triggered, this, &Player::actAlwaysLookAtTopCard);
        aOpenDeckInDeckEditor = new QAction(this);
        aOpenDeckInDeckEditor->setEnabled(false);
        connect(aOpenDeckInDeckEditor, &QAction::triggered, this, &Player::actOpenDeckInDeckEditor);
    }

    aViewGraveyard = new QAction(this);
    connect(aViewGraveyard, &QAction::triggered, this, &Player::actViewGraveyard);

    aViewRfg = new QAction(this);
    connect(aViewRfg, &QAction::triggered, this, &Player::actViewRfg);

    if (local || judge) {
        aViewSideboard = new QAction(this);
        connect(aViewSideboard, &QAction::triggered, this, &Player::actViewSideboard);

        aDrawCard = new QAction(this);
        connect(aDrawCard, &QAction::triggered, this, &Player::actDrawCard);
        aDrawCards = new QAction(this);
        connect(aDrawCards, &QAction::triggered, this, &Player::actDrawCards);
        aUndoDraw = new QAction(this);
        connect(aUndoDraw, &QAction::triggered, this, &Player::actUndoDraw);

        aShuffle = new QAction(this);
        connect(aShuffle, &QAction::triggered, this, &Player::actShuffle);

        aMulligan = new QAction(this);
        connect(aMulligan, &QAction::triggered, this, &Player::actMulligan);

        aMoveTopToPlay = new QAction(this);
        connect(aMoveTopToPlay, &QAction::triggered, this, &Player::actMoveTopCardToPlay);
        aMoveTopToPlayFaceDown = new QAction(this);
        connect(aMoveTopToPlayFaceDown, &QAction::triggered, this, &Player::actMoveTopCardToPlayFaceDown);
        aMoveTopCardToGraveyard = new QAction(this);
        connect(aMoveTopCardToGraveyard, &QAction::triggered, this, &Player::actMoveTopCardToGrave);
        aMoveTopCardToExile = new QAction(this);
        connect(aMoveTopCardToExile, &QAction::triggered, this, &Player::actMoveTopCardToExile);
        aMoveTopCardsToGraveyard = new QAction(this);
        connect(aMoveTopCardsToGraveyard, &QAction::triggered, this, &Player::actMoveTopCardsToGrave);
        aMoveTopCardsToExile = new QAction(this);
        connect(aMoveTopCardsToExile, &QAction::triggered, this, &Player::actMoveTopCardsToExile);
        aMoveTopCardsUntil = new QAction(this);
        connect(aMoveTopCardsUntil, &QAction::triggered, this, &Player::actMoveTopCardsUntil);
        aMoveTopCardToBottom = new QAction(this);
        connect(aMoveTopCardToBottom, &QAction::triggered, this, &Player::actMoveTopCardToBottom);

        aShuffleTopCards = new QAction(this);
        connect(aShuffleTopCards, &QAction::triggered, this, &Player::actShuffleTop);

        aDrawBottomCard = new QAction(this);
        connect(aDrawBottomCard, &QAction::triggered, this, &Player::actDrawBottomCard);
        aDrawBottomCards = new QAction(this);
        connect(aDrawBottomCards, &QAction::triggered, this, &Player::actDrawBottomCards);
        aMoveBottomToPlay = new QAction(this);
        connect(aMoveBottomToPlay, &QAction::triggered, this, &Player::actMoveBottomCardToPlay);
        aMoveBottomToPlayFaceDown = new QAction(this);
        connect(aMoveBottomToPlayFaceDown, &QAction::triggered, this, &Player::actMoveBottomCardToPlayFaceDown);
        aMoveBottomCardToGraveyard = new QAction(this);
        connect(aMoveBottomCardToGraveyard, &QAction::triggered, this, &Player::actMoveBottomCardToGrave);
        aMoveBottomCardToExile = new QAction(this);
        connect(aMoveBottomCardToExile, &QAction::triggered, this, &Player::actMoveBottomCardToExile);
        aMoveBottomCardsToGraveyard = new QAction(this);
        connect(aMoveBottomCardsToGraveyard, &QAction::triggered, this, &Player::actMoveBottomCardsToGrave);
        aMoveBottomCardsToExile = new QAction(this);
        connect(aMoveBottomCardsToExile, &QAction::triggered, this, &Player::actMoveBottomCardsToExile);
        aMoveBottomCardToTop = new QAction(this);
        connect(aMoveBottomCardToTop, &QAction::triggered, this, &Player::actMoveBottomCardToTop);

        aShuffleBottomCards = new QAction(this);
        connect(aShuffleBottomCards, &QAction::triggered, this, &Player::actShuffleBottom);
    }

    playerMenu = new TearOffMenu();
    table->setMenu(playerMenu);

    if (local || judge) {
        handMenu = playerMenu->addTearOffMenu(QString());
        handMenu->addAction(aViewHand);
        handMenu->addAction(aSortHand);
        playerLists.append(mRevealHand = handMenu->addMenu(QString()));
        playerLists.append(mRevealRandomHandCard = handMenu->addMenu(QString()));
        handMenu->addSeparator();
        handMenu->addAction(aMulligan);
        handMenu->addSeparator();
        moveHandMenu = handMenu->addTearOffMenu(QString());
        moveHandMenu->addAction(aMoveHandToTopLibrary);
        moveHandMenu->addAction(aMoveHandToBottomLibrary);
        moveHandMenu->addSeparator();
        moveHandMenu->addAction(aMoveHandToGrave);
        moveHandMenu->addSeparator();
        moveHandMenu->addAction(aMoveHandToRfg);
        hand->setMenu(handMenu);

        libraryMenu = playerMenu->addTearOffMenu(QString());
        libraryMenu->addAction(aDrawCard);
        libraryMenu->addAction(aDrawCards);
        libraryMenu->addAction(aUndoDraw);
        libraryMenu->addSeparator();
        libraryMenu->addAction(aShuffle);
        libraryMenu->addSeparator();
        libraryMenu->addAction(aViewLibrary);
        libraryMenu->addAction(aViewTopCards);
        libraryMenu->addAction(aViewBottomCards);
        libraryMenu->addSeparator();
        playerLists.append(mRevealLibrary = libraryMenu->addMenu(QString()));
        singlePlayerLists.append(mLendLibrary = libraryMenu->addMenu(QString()));
        playerLists.append(mRevealTopCard = libraryMenu->addMenu(QString()));
        libraryMenu->addAction(aAlwaysRevealTopCard);
        libraryMenu->addAction(aAlwaysLookAtTopCard);
        libraryMenu->addSeparator();
        topLibraryMenu = libraryMenu->addTearOffMenu(QString());
        bottomLibraryMenu = libraryMenu->addTearOffMenu(QString());
        libraryMenu->addSeparator();
        libraryMenu->addAction(aOpenDeckInDeckEditor);
        _deck->setMenu(libraryMenu, aDrawCard);
        topLibraryMenu->addAction(aMoveTopToPlay);
        topLibraryMenu->addAction(aMoveTopToPlayFaceDown);
        topLibraryMenu->addAction(aMoveTopCardToBottom);
        topLibraryMenu->addSeparator();
        topLibraryMenu->addAction(aMoveTopCardToGraveyard);
        topLibraryMenu->addAction(aMoveTopCardsToGraveyard);
        topLibraryMenu->addAction(aMoveTopCardToExile);
        topLibraryMenu->addAction(aMoveTopCardsToExile);
        topLibraryMenu->addAction(aMoveTopCardsUntil);
        topLibraryMenu->addSeparator();
        topLibraryMenu->addAction(aShuffleTopCards);

        bottomLibraryMenu->addAction(aDrawBottomCard);
        bottomLibraryMenu->addAction(aDrawBottomCards);
        bottomLibraryMenu->addSeparator();
        bottomLibraryMenu->addAction(aMoveBottomToPlay);
        bottomLibraryMenu->addAction(aMoveBottomToPlayFaceDown);
        bottomLibraryMenu->addAction(aMoveBottomCardToTop);
        bottomLibraryMenu->addSeparator();
        bottomLibraryMenu->addAction(aMoveBottomCardToGraveyard);
        bottomLibraryMenu->addAction(aMoveBottomCardsToGraveyard);
        bottomLibraryMenu->addAction(aMoveBottomCardToExile);
        bottomLibraryMenu->addAction(aMoveBottomCardsToExile);
        bottomLibraryMenu->addSeparator();
        bottomLibraryMenu->addAction(aShuffleBottomCards);
    }

    graveMenu = playerMenu->addTearOffMenu(QString());
    graveMenu->addAction(aViewGraveyard);

    if (local || judge) {
        mRevealRandomGraveyardCard = graveMenu->addMenu(QString());
        QAction *newAction = mRevealRandomGraveyardCard->addAction(QString());
        newAction->setData(-1);
        connect(newAction, &QAction::triggered, this, &Player::actRevealRandomGraveyardCard);
        allPlayersActions.append(newAction);
        mRevealRandomGraveyardCard->addSeparator();
    }
    grave->setMenu(graveMenu, aViewGraveyard);

    rfgMenu = playerMenu->addTearOffMenu(QString());
    rfgMenu->addAction(aViewRfg);
    rfg->setMenu(rfgMenu, aViewRfg);

    if (local || judge) {
        graveMenu->addSeparator();
        moveGraveMenu = graveMenu->addTearOffMenu(QString());
        moveGraveMenu->addAction(aMoveGraveToTopLibrary);
        moveGraveMenu->addAction(aMoveGraveToBottomLibrary);
        moveGraveMenu->addSeparator();
        moveGraveMenu->addAction(aMoveGraveToHand);
        moveGraveMenu->addSeparator();
        moveGraveMenu->addAction(aMoveGraveToRfg);

        rfgMenu->addSeparator();
        moveRfgMenu = rfgMenu->addTearOffMenu(QString());
        moveRfgMenu->addAction(aMoveRfgToTopLibrary);
        moveRfgMenu->addAction(aMoveRfgToBottomLibrary);
        moveRfgMenu->addSeparator();
        moveRfgMenu->addAction(aMoveRfgToHand);
        moveRfgMenu->addSeparator();
        moveRfgMenu->addAction(aMoveRfgToGrave);

        sbMenu = playerMenu->addMenu(QString());
        sbMenu->addAction(aViewSideboard);
        sb->setMenu(sbMenu, aViewSideboard);

        mCustomZones = playerMenu->addMenu(QString());
        mCustomZones->menuAction()->setVisible(false);

        aUntapAll = new QAction(this);
        connect(aUntapAll, &QAction::triggered, this, &Player::actUntapAll);

        aRollDie = new QAction(this);
        connect(aRollDie, &QAction::triggered, this, &Player::actRollDie);

        aCreateToken = new QAction(this);
        connect(aCreateToken, &QAction::triggered, this, &Player::actCreateToken);

        aCreateAnotherToken = new QAction(this);
        connect(aCreateAnotherToken, &QAction::triggered, this, &Player::actCreateAnotherToken);
        aCreateAnotherToken->setEnabled(false);

        aIncrementAllCardCounters = new QAction(this);
        connect(aIncrementAllCardCounters, &QAction::triggered, this, &Player::incrementAllCardCounters);

        createPredefinedTokenMenu = new QMenu(QString());
        createPredefinedTokenMenu->setEnabled(false);

        mCardCounters = new QMenu;

        playerMenu->addSeparator();
        countersMenu = playerMenu->addMenu(QString());
        playerMenu->addAction(aIncrementAllCardCounters);
        playerMenu->addSeparator();
        playerMenu->addAction(aUntapAll);
        playerMenu->addSeparator();
        playerMenu->addAction(aRollDie);
        playerMenu->addSeparator();
        playerMenu->addAction(aCreateToken);
        playerMenu->addAction(aCreateAnotherToken);
        playerMenu->addMenu(createPredefinedTokenMenu);
        playerMenu->addSeparator();
    }

    if (local) {
        sayMenu = playerMenu->addMenu(QString());
        connect(&SettingsCache::instance().messages(), &MessageSettings::messageMacrosChanged, this,
                &Player::initSayMenu);
        initSayMenu();
    }

    aCardMenu = new QAction(this);
    aCardMenu->setEnabled(false);
    playerMenu->addSeparator();
    playerMenu->addAction(aCardMenu);

    if (local || judge) {

        for (auto &playerList : playerLists) {
            QAction *newAction = playerList->addAction(QString());
            newAction->setData(-1);
            connect(newAction, &QAction::triggered, this, &Player::playerListActionTriggered);
            allPlayersActions.append(newAction);
            playerList->addSeparator();
        }
    }

    if (!local && !judge) {
        countersMenu = nullptr;
        sbMenu = nullptr;
        mCustomZones = nullptr;
        aCreateAnotherToken = nullptr;
        createPredefinedTokenMenu = nullptr;
        mCardCounters = nullptr;
    }

    aTap = new QAction(this);
    aTap->setData(cmTap);
    connect(aTap, &QAction::triggered, this, &Player::cardMenuAction);
    aDoesntUntap = new QAction(this);
    aDoesntUntap->setData(cmDoesntUntap);
    connect(aDoesntUntap, &QAction::triggered, this, &Player::cardMenuAction);
    aAttach = new QAction(this);
    connect(aAttach, &QAction::triggered, this, &Player::actAttach);
    aUnattach = new QAction(this);
    connect(aUnattach, &QAction::triggered, this, &Player::actUnattach);
    aDrawArrow = new QAction(this);
    connect(aDrawArrow, &QAction::triggered, this, &Player::actDrawArrow);
    aIncP = new QAction(this);
    connect(aIncP, &QAction::triggered, this, &Player::actIncP);
    aDecP = new QAction(this);
    connect(aDecP, &QAction::triggered, this, &Player::actDecP);
    aIncT = new QAction(this);
    connect(aIncT, &QAction::triggered, this, &Player::actIncT);
    aDecT = new QAction(this);
    connect(aDecT, &QAction::triggered, this, &Player::actDecT);
    aIncPT = new QAction(this);
    connect(aIncPT, &QAction::triggered, this, [this] { actIncPT(); });
    aDecPT = new QAction(this);
    connect(aDecPT, &QAction::triggered, this, &Player::actDecPT);
    aFlowP = new QAction(this);
    connect(aFlowP, &QAction::triggered, this, &Player::actFlowP);
    aFlowT = new QAction(this);
    connect(aFlowT, &QAction::triggered, this, &Player::actFlowT);
    aSetPT = new QAction(this);
    connect(aSetPT, &QAction::triggered, this, &Player::actSetPT);
    aResetPT = new QAction(this);
    connect(aResetPT, &QAction::triggered, this, &Player::actResetPT);
    aSetAnnotation = new QAction(this);
    connect(aSetAnnotation, &QAction::triggered, this, &Player::actSetAnnotation);
    aFlip = new QAction(this);
    aFlip->setData(cmFlip);
    connect(aFlip, &QAction::triggered, this, &Player::cardMenuAction);
    aPeek = new QAction(this);
    aPeek->setData(cmPeek);
    connect(aPeek, &QAction::triggered, this, &Player::cardMenuAction);
    aClone = new QAction(this);
    aClone->setData(cmClone);
    connect(aClone, &QAction::triggered, this, &Player::cardMenuAction);
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
    connect(aMoveToTopLibrary, &QAction::triggered, this, &Player::cardMenuAction);
    connect(aMoveToBottomLibrary, &QAction::triggered, this, &Player::cardMenuAction);
    connect(aMoveToXfromTopOfLibrary, &QAction::triggered, this, &Player::actMoveCardXCardsFromTop);
    connect(aMoveToHand, &QAction::triggered, this, &Player::cardMenuAction);
    connect(aMoveToGraveyard, &QAction::triggered, this, &Player::cardMenuAction);
    connect(aMoveToExile, &QAction::triggered, this, &Player::cardMenuAction);

    aSelectAll = new QAction(this);
    connect(aSelectAll, &QAction::triggered, this, &Player::actSelectAll);
    aSelectRow = new QAction(this);
    connect(aSelectRow, &QAction::triggered, this, &Player::actSelectRow);
    aSelectColumn = new QAction(this);
    connect(aSelectColumn, &QAction::triggered, this, &Player::actSelectColumn);

    aPlay = new QAction(this);
    connect(aPlay, &QAction::triggered, this, &Player::actPlay);
    aHide = new QAction(this);
    connect(aHide, &QAction::triggered, this, &Player::actHide);
    aPlayFacedown = new QAction(this);
    connect(aPlayFacedown, &QAction::triggered, this, &Player::actPlayFacedown);

    for (int i = 0; i < 6; ++i) {
        auto *tempAddCounter = new QAction(this);
        tempAddCounter->setData(9 + i * 1000);
        auto *tempRemoveCounter = new QAction(this);
        tempRemoveCounter->setData(10 + i * 1000);
        auto *tempSetCounter = new QAction(this);
        tempSetCounter->setData(11 + i * 1000);
        aAddCounter.append(tempAddCounter);
        aRemoveCounter.append(tempRemoveCounter);
        aSetCounter.append(tempSetCounter);
        connect(tempAddCounter, &QAction::triggered, this, &Player::actCardCounterTrigger);
        connect(tempRemoveCounter, &QAction::triggered, this, &Player::actCardCounterTrigger);
        connect(tempSetCounter, &QAction::triggered, this, &Player::actCardCounterTrigger);
    }

    const QList<Player *> &players = game->getPlayers().values();
    for (const auto player : players) {
        addPlayer(player);
    }

    moveTopCardTimer = new QTimer(this);
    moveTopCardTimer->setInterval(MOVE_TOP_CARD_UNTIL_INTERVAL);
    moveTopCardTimer->setSingleShot(true);
    connect(moveTopCardTimer, &QTimer::timeout, [this]() { actMoveTopCardToPlay(); });

    rearrangeZones();
    retranslateUi();
    connect(&SettingsCache::instance().shortcuts(), &ShortcutsSettings::shortCutChanged, this,
            &Player::refreshShortcuts);
    refreshShortcuts();
}

Player::~Player()
{
    qCInfo(PlayerLog) << "Player destructor:" << getName();

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
        addPlayerToList(playerList, player);
    }
    for (auto &playerList : singlePlayerLists) {
        addPlayerToList(playerList, player);
    }

    playersInfo.append(qMakePair(player->getName(), player->getId()));
}

void Player::addPlayerToList(QMenu *playerList, Player *player)
{
    QAction *newAction = playerList->addAction(player->getName());
    newAction->setData(player->getId());
    connect(newAction, &QAction::triggered, this, &Player::playerListActionTriggered);
}

void Player::removePlayer(Player *player)
{
    if (player == nullptr) {
        return;
    }

    for (auto &playerList : playerLists) {
        removePlayerFromList(playerList, player);
    }
    for (auto &playerList : singlePlayerLists) {
        removePlayerFromList(playerList, player);
    }

    for (auto it = playersInfo.begin(); it != playersInfo.end();) {
        if (it->second == player->getId()) {
            it = playersInfo.erase(it);
        } else {
            ++it;
        }
    }
}

void Player::removePlayerFromList(QMenu *playerList, Player *player)
{
    QList<QAction *> actionList = playerList->actions();
    for (auto &j : actionList)
        if (j->data().toInt() == player->getId()) {
            playerList->removeAction(j);
            j->deleteLater();
        }
}

void Player::playerListActionTriggered()
{
    auto *action = static_cast<QAction *>(sender());
    auto *menu = static_cast<QMenu *>(action->parent());

    Command_RevealCards cmd;
    const int otherPlayerId = action->data().toInt();
    if (otherPlayerId != -1) {
        cmd.set_player_id(otherPlayerId);
    }

    if (menu == mRevealLibrary || menu == mLendLibrary) {
        cmd.set_zone_name("deck");
        cmd.set_grant_write_access(menu == mLendLibrary);
    } else if (menu == mRevealTopCard) {
        int deckSize = zones.value("deck")->getCards().size();
        bool ok;
        int number = QInputDialog::getInt(game, tr("Reveal top cards of library"),
                                          tr("Number of cards: (max. %1)").arg(deckSize), defaultNumberTopCards, 1,
                                          deckSize, 1, &ok);
        if (ok) {
            cmd.set_zone_name("deck");
            cmd.set_top_cards(number);
            // backward compatibility: servers before #1051 only permits to reveal the first card
            cmd.add_card_id(0);
            defaultNumberTopCards = number;
        }
    } else if (menu == mRevealHand) {
        cmd.set_zone_name("hand");
    } else if (menu == mRevealRandomHandCard) {
        cmd.set_zone_name("hand");
        cmd.add_card_id(RANDOM_CARD_FROM_ZONE);
    } else {
        return;
    }

    sendGameCommand(cmd);
}

void Player::rearrangeZones()
{
    QPointF base = QPointF(CARD_HEIGHT + counterAreaWidth + 15, 0);
    if (SettingsCache::instance().getHorizontalHand()) {
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
    if (SettingsCache::instance().getHorizontalHand()) {
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

    if (local || judge) {
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
        aViewHand->setText(tr("&View hand"));
        aSortHand->setText(tr("&Sort hand"));
        aViewTopCards->setText(tr("View &top cards of library..."));
        aViewBottomCards->setText(tr("View bottom cards of library..."));
        mRevealLibrary->setTitle(tr("Reveal &library to..."));
        mLendLibrary->setTitle(tr("Lend library to..."));
        mRevealTopCard->setTitle(tr("Reveal &top cards to..."));
        topLibraryMenu->setTitle(tr("&Top of library..."));
        bottomLibraryMenu->setTitle(tr("&Bottom of library..."));
        aAlwaysRevealTopCard->setText(tr("&Always reveal top card"));
        aAlwaysLookAtTopCard->setText(tr("&Always look at top card"));
        aOpenDeckInDeckEditor->setText(tr("&Open deck in deck editor"));
        aViewSideboard->setText(tr("&View sideboard"));
        aDrawCard->setText(tr("&Draw card"));
        aDrawCards->setText(tr("D&raw cards..."));
        aUndoDraw->setText(tr("&Undo last draw"));
        aMulligan->setText(tr("Take &mulligan"));

        aShuffle->setText(tr("Shuffle"));

        aMoveTopToPlay->setText(tr("&Play top card"));
        aMoveTopToPlayFaceDown->setText(tr("Play top card &face down"));
        aMoveTopCardToBottom->setText(tr("Put top card on &bottom"));
        aMoveTopCardToGraveyard->setText(tr("Move top card to grave&yard"));
        aMoveTopCardToExile->setText(tr("Move top card to e&xile"));
        aMoveTopCardsToGraveyard->setText(tr("Move top cards to &graveyard..."));
        aMoveTopCardsToExile->setText(tr("Move top cards to &exile..."));
        aMoveTopCardsUntil->setText(tr("Put top cards on stack &until..."));
        aShuffleTopCards->setText(tr("Shuffle top cards..."));

        aDrawBottomCard->setText(tr("&Draw bottom card"));
        aDrawBottomCards->setText(tr("D&raw bottom cards..."));
        aMoveBottomToPlay->setText(tr("&Play bottom card"));
        aMoveBottomToPlayFaceDown->setText(tr("Play bottom card &face down"));
        aMoveBottomCardToGraveyard->setText(tr("Move bottom card to grave&yard"));
        aMoveBottomCardToExile->setText(tr("Move bottom card to e&xile"));
        aMoveBottomCardsToGraveyard->setText(tr("Move bottom cards to &graveyard..."));
        aMoveBottomCardsToExile->setText(tr("Move bottom cards to &exile..."));
        aMoveBottomCardToTop->setText(tr("Put bottom card on &top"));
        aShuffleBottomCards->setText(tr("Shuffle bottom cards..."));

        handMenu->setTitle(tr("&Hand"));
        mRevealHand->setTitle(tr("&Reveal hand to..."));
        mRevealRandomHandCard->setTitle(tr("Reveal r&andom card to..."));
        mRevealRandomGraveyardCard->setTitle(tr("Reveal random card to..."));
        sbMenu->setTitle(tr("&Sideboard"));
        libraryMenu->setTitle(tr("&Library"));
        countersMenu->setTitle(tr("&Counters"));
        mCustomZones->setTitle(tr("C&ustom Zones"));

        for (auto aViewZone : mCustomZones->actions()) {
            aViewZone->setText(tr("View custom zone '%1'").arg(aViewZone->data().toString()));
        }

        aIncrementAllCardCounters->setText(tr("Increment all card counters"));
        aUntapAll->setText(tr("&Untap all permanents"));
        aRollDie->setText(tr("R&oll die..."));
        aCreateToken->setText(tr("&Create token..."));
        aCreateAnotherToken->setText(tr("C&reate another token"));
        createPredefinedTokenMenu->setTitle(tr("Cr&eate predefined token"));

        mCardCounters->setTitle(tr("Ca&rd counters"));

        QMapIterator<int, AbstractCounter *> counterIterator(counters);
        while (counterIterator.hasNext()) {
            counterIterator.next().value()->retranslateUi();
        }

        for (auto &allPlayersAction : allPlayersActions) {
            allPlayersAction->setText(tr("&All players"));
        }
    }

    aCardMenu->setText(tr("Selec&ted cards"));

    if (local) {
        sayMenu->setTitle(tr("S&ay"));
    }

    aSelectAll->setText(tr("&Select All"));
    aSelectRow->setText(tr("S&elect Row"));
    aSelectColumn->setText(tr("S&elect Column"));

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
    aFlowP->setText(tr("Increase power and decrease toughness"));
    aFlowT->setText(tr("Decrease power and increase toughness"));
    aSetPT->setText(tr("Set &power and toughness..."));
    aResetPT->setText(tr("Reset p&ower and toughness"));
    aSetAnnotation->setText(tr("&Set annotation..."));

    auto &cardCounterSettings = SettingsCache::instance().cardCounters();

    for (int i = 0; i < aAddCounter.size(); ++i) {
        aAddCounter[i]->setText(tr("&Add counter (%1)").arg(cardCounterSettings.displayName(i)));
    }
    for (int i = 0; i < aRemoveCounter.size(); ++i) {
        aRemoveCounter[i]->setText(tr("&Remove counter (%1)").arg(cardCounterSettings.displayName(i)));
    }
    for (int i = 0; i < aSetCounter.size(); ++i) {
        aSetCounter[i]->setText(tr("&Set counters (%1)...").arg(cardCounterSettings.displayName(i)));
    }

    aMoveToTopLibrary->setText(tr("&Top of library in random order"));
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
    ShortcutsSettings &shortcuts = SettingsCache::instance().shortcuts();

    aHide->setShortcuts(shortcuts.getShortcut("Player/aHide"));
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
    aFlowP->setShortcuts(shortcuts.getShortcut("Player/aFlowP"));
    aFlowT->setShortcuts(shortcuts.getShortcut("Player/aFlowT"));
    aSetPT->setShortcuts(shortcuts.getShortcut("Player/aSetPT"));
    aResetPT->setShortcuts(shortcuts.getShortcut("Player/aResetPT"));
    aSetAnnotation->setShortcuts(shortcuts.getShortcut("Player/aSetAnnotation"));
    aMoveToTopLibrary->setShortcuts(shortcuts.getShortcut("Player/aMoveToTopLibrary"));
    aMoveToBottomLibrary->setShortcuts(shortcuts.getShortcut("Player/aMoveToBottomLibrary"));
    aMoveToHand->setShortcuts(shortcuts.getShortcut("Player/aMoveToHand"));
    aMoveToGraveyard->setShortcuts(shortcuts.getShortcut("Player/aMoveToGraveyard"));
    aMoveToExile->setShortcuts(shortcuts.getShortcut("Player/aMoveToExile"));
    aSortHand->setShortcuts(shortcuts.getShortcut("Player/aSortHand"));

    aSelectAll->setShortcuts(shortcuts.getShortcut("Player/aSelectAll"));
    aSelectRow->setShortcuts(shortcuts.getShortcut("Player/aSelectRow"));
    aSelectColumn->setShortcuts(shortcuts.getShortcut("Player/aSelectColumn"));

    static const QStringList colorWords = {"Red", "Yellow", "Green", "Cyan", "Purple", "Magenta"};
    for (int i = 0; i < aAddCounter.size(); i++) {
        aAddCounter[i]->setShortcuts(shortcuts.getShortcut("Player/aCC" + colorWords[i]));
        aRemoveCounter[i]->setShortcuts(shortcuts.getShortcut("Player/aRC" + colorWords[i]));
        aSetCounter[i]->setShortcuts(shortcuts.getShortcut("Player/aSC" + colorWords[i]));
    }

    QMapIterator<int, AbstractCounter *> counterIterator(counters);
    while (counterIterator.hasNext()) {
        counterIterator.next().value()->setShortcutsActive();
    }

    aIncrementAllCardCounters->setShortcuts(shortcuts.getShortcut("Player/aIncrementAllCardCounters"));
    aViewSideboard->setShortcuts(shortcuts.getShortcut("Player/aViewSideboard"));
    aViewLibrary->setShortcuts(shortcuts.getShortcut("Player/aViewLibrary"));
    aViewHand->setShortcuts(shortcuts.getShortcut("Player/aViewHand"));
    aViewTopCards->setShortcuts(shortcuts.getShortcut("Player/aViewTopCards"));
    aViewBottomCards->setShortcuts(shortcuts.getShortcut("Player/aViewBottomCards"));
    aViewGraveyard->setShortcuts(shortcuts.getShortcut("Player/aViewGraveyard"));
    aDrawCard->setShortcuts(shortcuts.getShortcut("Player/aDrawCard"));
    aDrawCards->setShortcuts(shortcuts.getShortcut("Player/aDrawCards"));
    aUndoDraw->setShortcuts(shortcuts.getShortcut("Player/aUndoDraw"));
    aMulligan->setShortcuts(shortcuts.getShortcut("Player/aMulligan"));
    aShuffle->setShortcuts(shortcuts.getShortcut("Player/aShuffle"));
    aShuffleTopCards->setShortcuts(shortcuts.getShortcut("Player/aShuffleTopCards"));
    aShuffleBottomCards->setShortcuts(shortcuts.getShortcut("Player/aShuffleBottomCards"));
    aUntapAll->setShortcuts(shortcuts.getShortcut("Player/aUntapAll"));
    aRollDie->setShortcuts(shortcuts.getShortcut("Player/aRollDie"));
    aCreateToken->setShortcuts(shortcuts.getShortcut("Player/aCreateToken"));
    aCreateAnotherToken->setShortcuts(shortcuts.getShortcut("Player/aCreateAnotherToken"));
    aAlwaysRevealTopCard->setShortcuts(shortcuts.getShortcut("Player/aAlwaysRevealTopCard"));
    aAlwaysLookAtTopCard->setShortcuts(shortcuts.getShortcut("Player/aAlwaysLookAtTopCard"));
    aMoveTopToPlay->setShortcuts(shortcuts.getShortcut("Player/aMoveTopToPlay"));
    aMoveTopToPlayFaceDown->setShortcuts(shortcuts.getShortcut("Player/aMoveTopToPlayFaceDown"));
    aMoveTopCardToGraveyard->setShortcuts(shortcuts.getShortcut("Player/aMoveTopCardToGraveyard"));
    aMoveTopCardsToGraveyard->setShortcuts(shortcuts.getShortcut("Player/aMoveTopCardsToGraveyard"));
    aMoveTopCardToExile->setShortcuts(shortcuts.getShortcut("Player/aMoveTopCardToExile"));
    aMoveTopCardsToExile->setShortcuts(shortcuts.getShortcut("Player/aMoveTopCardsToExile"));
    aMoveTopCardsUntil->setShortcuts(shortcuts.getShortcut("Player/aMoveTopCardsUntil"));
    aMoveTopCardToBottom->setShortcuts(shortcuts.getShortcut("Player/aMoveTopCardToBottom"));
    aDrawBottomCard->setShortcuts(shortcuts.getShortcut("Player/aDrawBottomCard"));
    aDrawBottomCards->setShortcuts(shortcuts.getShortcut("Player/aDrawBottomCards"));
    aMoveBottomToPlay->setShortcuts(shortcuts.getShortcut("Player/aMoveBottomToPlay"));
    aMoveBottomToPlayFaceDown->setShortcuts(shortcuts.getShortcut("Player/aMoveBottomToPlayFaceDown"));
    aMoveBottomCardToGraveyard->setShortcuts(shortcuts.getShortcut("Player/aMoveBottomCardToGrave"));
    aMoveBottomCardsToGraveyard->setShortcuts(shortcuts.getShortcut("Player/aMoveBottomCardsToGrave"));
    aMoveBottomCardToExile->setShortcuts(shortcuts.getShortcut("Player/aMoveBottomCardToExile"));
    aMoveBottomCardsToExile->setShortcuts(shortcuts.getShortcut("Player/aMoveBottomCardsToExile"));
    aMoveBottomCardToTop->setShortcuts(shortcuts.getShortcut("Player/aMoveBottomCardToTop"));
    aPlayFacedown->setShortcuts(shortcuts.getShortcut("Player/aPlayFacedown"));
    aPlay->setShortcuts(shortcuts.getShortcut("Player/aPlay"));

    // Don't enable always-active shortcuts in local games, since it causes keyboard shortcuts to work inconsistently
    // when there are more than 1 player.
    if (!game->getIsLocalGame()) {
        // unattach action is only active in card menu if the active card is attached.
        // make unattach shortcut always active so that it consistently works when multiple cards are selected.
        game->addAction(aUnattach);
    }
}

void Player::setShortcutsInactive()
{
    shortcutsActive = false;

    aViewSideboard->setShortcut(QKeySequence());
    aViewLibrary->setShortcut(QKeySequence());
    aViewHand->setShortcut(QKeySequence());
    aViewTopCards->setShortcut(QKeySequence());
    aViewBottomCards->setShortcut(QKeySequence());
    aViewGraveyard->setShortcut(QKeySequence());
    aDrawCard->setShortcut(QKeySequence());
    aDrawCards->setShortcut(QKeySequence());
    aUndoDraw->setShortcut(QKeySequence());
    aMulligan->setShortcut(QKeySequence());
    aShuffle->setShortcut(QKeySequence());
    aShuffleTopCards->setShortcut(QKeySequence());
    aShuffleBottomCards->setShortcut(QKeySequence());
    aUntapAll->setShortcut(QKeySequence());
    aRollDie->setShortcut(QKeySequence());
    aCreateToken->setShortcut(QKeySequence());
    aCreateAnotherToken->setShortcut(QKeySequence());
    aAlwaysRevealTopCard->setShortcut(QKeySequence());
    aAlwaysLookAtTopCard->setShortcut(QKeySequence());
    aMoveTopToPlay->setShortcut(QKeySequence());
    aMoveTopToPlayFaceDown->setShortcut(QKeySequence());
    aMoveTopCardToGraveyard->setShortcut(QKeySequence());
    aMoveTopCardsToGraveyard->setShortcut(QKeySequence());
    aMoveTopCardToExile->setShortcut(QKeySequence());
    aMoveTopCardsToExile->setShortcut(QKeySequence());
    aMoveTopCardsUntil->setShortcut(QKeySequence());
    aDrawBottomCard->setShortcut(QKeySequence());
    aDrawBottomCards->setShortcut(QKeySequence());
    aMoveBottomToPlay->setShortcut(QKeySequence());
    aMoveBottomToPlayFaceDown->setShortcut(QKeySequence());
    aMoveBottomCardToGraveyard->setShortcut(QKeySequence());
    aMoveBottomCardsToGraveyard->setShortcut(QKeySequence());
    aMoveBottomCardToExile->setShortcut(QKeySequence());
    aMoveBottomCardsToExile->setShortcut(QKeySequence());
    aIncrementAllCardCounters->setShortcut(QKeySequence());
    aSortHand->setShortcut(QKeySequence());

    QMapIterator<int, AbstractCounter *> counterIterator(counters);
    while (counterIterator.hasNext()) {
        counterIterator.next().value()->setShortcutsInactive();
    }
}

void Player::initSayMenu()
{
    sayMenu->clear();

    int count = SettingsCache::instance().messages().getCount();
    sayMenu->setEnabled(count > 0);

    for (int i = 0; i < count; ++i) {
        auto *newAction = new QAction(SettingsCache::instance().messages().getMessageAt(i), sayMenu);
        if (i < 10) {
            newAction->setShortcut(QKeySequence("Ctrl+" + QString::number((i + 1) % 10)));
        }
        connect(newAction, &QAction::triggered, this, &Player::actSayMessage);
        sayMenu->addAction(newAction);
    }
}

void Player::initContextualPlayersMenu(QMenu *menu)
{
    menu->addAction(tr("&All players"))->setData(-1);
    menu->addSeparator();

    for (const auto &playerInfo : playersInfo) {
        menu->addAction(playerInfo.first)->setData(playerInfo.second);
    }
}

void Player::setDeck(const DeckLoader &_deck)
{
    deck = new DeckLoader(_deck);
    aOpenDeckInDeckEditor->setEnabled(deck);

    createPredefinedTokenMenu->clear();
    createPredefinedTokenMenu->setEnabled(false);
    predefinedTokens.clear();
    InnerDecklistNode *tokenZone = dynamic_cast<InnerDecklistNode *>(deck->getRoot()->findChild(DECK_ZONE_TOKENS));

    if (tokenZone) {
        if (tokenZone->size() > 0)
            createPredefinedTokenMenu->setEnabled(true);

        for (int i = 0; i < tokenZone->size(); ++i) {
            const QString tokenName = tokenZone->at(i)->getName();
            predefinedTokens.append(tokenName);
            QAction *a = createPredefinedTokenMenu->addAction(tokenName);
            if (i < 10) {
                a->setShortcut(QKeySequence("Alt+" + QString::number((i + 1) % 10)));
            }
            connect(a, &QAction::triggered, this, &Player::actCreatePredefinedToken);
        }
    }
}

void Player::actViewLibrary()
{
    static_cast<GameScene *>(scene())->toggleZoneView(this, "deck", -1);
}

void Player::actViewHand()
{
    static_cast<GameScene *>(scene())->toggleZoneView(this, "hand", -1);
}

void Player::actSortHand()
{
    hand->sortHand();
}

void Player::actViewTopCards()
{
    int deckSize = zones.value("deck")->getCards().size();
    bool ok;
    int number =
        QInputDialog::getInt(game, tr("View top cards of library"), tr("Number of cards: (max. %1)").arg(deckSize),
                             defaultNumberTopCards, 1, deckSize, 1, &ok);
    if (ok) {
        defaultNumberTopCards = number;
        static_cast<GameScene *>(scene())->toggleZoneView(this, "deck", number);
    }
}

void Player::actViewBottomCards()
{
    int deckSize = zones.value("deck")->getCards().size();
    bool ok;
    int number =
        QInputDialog::getInt(game, tr("View bottom cards of library"), tr("Number of cards: (max. %1)").arg(deckSize),
                             defaultNumberBottomCards, 1, deckSize, 1, &ok);
    if (ok) {
        defaultNumberBottomCards = number;
        static_cast<GameScene *>(scene())->toggleZoneView(this, "deck", number, true);
    }
}

void Player::actAlwaysRevealTopCard()
{
    Command_ChangeZoneProperties cmd;
    cmd.set_zone_name("deck");
    cmd.set_always_reveal_top_card(aAlwaysRevealTopCard->isChecked());

    sendGameCommand(cmd);
}

void Player::actAlwaysLookAtTopCard()
{
    Command_ChangeZoneProperties cmd;
    cmd.set_zone_name("deck");
    cmd.set_always_look_at_top_card(aAlwaysLookAtTopCard->isChecked());

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
    cmd.add_card_id(RANDOM_CARD_FROM_ZONE);
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

void Player::actShuffleTop()
{
    const int maxCards = zones.value("deck")->getCards().size();
    if (maxCards == 0) {
        return;
    }

    bool ok;
    int number =
        QInputDialog::getInt(game, tr("Shuffle top cards of library"), tr("Number of cards: (max. %1)").arg(maxCards),
                             defaultNumberTopCards, 1, maxCards, 1, &ok);
    if (!ok) {
        return;
    }

    if (number > maxCards) {
        number = maxCards;
    }

    defaultNumberTopCards = number;

    Command_Shuffle cmd;
    cmd.set_zone_name("deck");
    cmd.set_start(0);
    cmd.set_end(number - 1); // inclusive, the indexed card at end will be shuffled

    sendGameCommand(cmd);
}

void Player::actShuffleBottom()
{
    const int maxCards = zones.value("deck")->getCards().size();
    if (maxCards == 0) {
        return;
    }

    bool ok;
    int number = QInputDialog::getInt(game, tr("Shuffle bottom cards of library"),
                                      tr("Number of cards: (max. %1)").arg(maxCards), defaultNumberBottomCards, 1,
                                      maxCards, 1, &ok);
    if (!ok) {
        return;
    }

    if (number > maxCards) {
        number = maxCards;
    }

    defaultNumberBottomCards = number;

    Command_Shuffle cmd;
    cmd.set_zone_name("deck");
    cmd.set_start(-number);
    cmd.set_end(-1);

    sendGameCommand(cmd);
}

void Player::actDrawCard()
{
    Command_DrawCards cmd;
    cmd.set_number(1);
    sendGameCommand(cmd);
}

void Player::actMulligan()
{
    int startSize = SettingsCache::instance().getStartingHandSize();
    int handSize = zones.value("hand")->getCards().size();
    int deckSize = zones.value("deck")->getCards().size() + handSize; // hand is shuffled back into the deck
    bool ok;
    int number = QInputDialog::getInt(game, tr("Draw hand"),
                                      tr("Number of cards: (max. %1)").arg(deckSize) + '\n' +
                                          tr("0 and lower are in comparison to current hand size"),
                                      startSize, -handSize, deckSize, 1, &ok);
    if (!ok) {
        return;
    }
    Command_Mulligan cmd;
    if (number < 1) {
        if (handSize == 0) {
            return;
        }
        cmd.set_number(handSize + number);
    } else {
        cmd.set_number(number);
    }
    sendGameCommand(cmd);
    if (startSize != number) {
        SettingsCache::instance().setStartingHandSize(number);
    }
}

void Player::actDrawCards()
{
    int deckSize = zones.value("deck")->getCards().size();
    bool ok;
    int number = QInputDialog::getInt(game, tr("Draw cards"), tr("Number of cards: (max. %1)").arg(deckSize),
                                      defaultNumberTopCards, 1, deckSize, 1, &ok);
    if (ok) {
        defaultNumberTopCards = number;
        Command_DrawCards cmd;
        cmd.set_number(static_cast<google::protobuf::uint32>(number));
        sendGameCommand(cmd);
    }
}

void Player::actUndoDraw()
{
    sendGameCommand(Command_UndoDraw());
}

void Player::cmdSetTopCard(Command_MoveCard &cmd)
{
    cmd.set_start_zone("deck");
    auto *cardToMove = cmd.mutable_cards_to_move()->add_card();
    cardToMove->set_card_id(0);
    cmd.set_target_player_id(getId());
}

void Player::cmdSetBottomCard(Command_MoveCard &cmd)
{
    CardZone *zone = zones.value("deck");
    int lastCard = zone->getCards().size() - 1;
    cmd.set_start_zone("deck");
    auto *cardToMove = cmd.mutable_cards_to_move()->add_card();
    cardToMove->set_card_id(lastCard);
    cmd.set_target_player_id(getId());
}

void Player::actMoveTopCardToGrave()
{
    if (zones.value("deck")->getCards().empty()) {
        return;
    }

    Command_MoveCard cmd;
    cmdSetTopCard(cmd);
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
    cmdSetTopCard(cmd);
    cmd.set_target_zone("rfg");
    cmd.set_x(0);
    cmd.set_y(0);

    sendGameCommand(cmd);
}

void Player::actMoveTopCardsToGrave()
{
    const int maxCards = zones.value("deck")->getCards().size();
    if (maxCards == 0) {
        return;
    }

    bool ok;
    int number =
        QInputDialog::getInt(game, tr("Move top cards to grave"), tr("Number of cards: (max. %1)").arg(maxCards),
                             defaultNumberTopCards, 1, maxCards, 1, &ok);
    if (!ok) {
        return;
    } else if (number > maxCards) {
        number = maxCards;
    }
    defaultNumberTopCards = number;

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
    const int maxCards = zones.value("deck")->getCards().size();
    if (maxCards == 0) {
        return;
    }

    bool ok;
    int number =
        QInputDialog::getInt(game, tr("Move top cards to exile"), tr("Number of cards: (max. %1)").arg(maxCards),
                             defaultNumberTopCards, 1, maxCards, 1, &ok);
    if (!ok) {
        return;
    } else if (number > maxCards) {
        number = maxCards;
    }
    defaultNumberTopCards = number;

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

void Player::actMoveTopCardsUntil()
{
    stopMoveTopCardsUntil();

    DlgMoveTopCardsUntil dlg(game, movingCardsUntilExprs, movingCardsUntilNumberOfHits, movingCardsUntilAutoPlay);
    if (!dlg.exec()) {
        return;
    }

    auto expr = dlg.getExpr();
    movingCardsUntilExprs = dlg.getExprs();
    movingCardsUntilNumberOfHits = dlg.getNumberOfHits();
    movingCardsUntilAutoPlay = dlg.isAutoPlay();

    if (zones.value("deck")->getCards().empty()) {
        stopMoveTopCardsUntil();
    } else {
        movingCardsUntilFilter = FilterString(expr);
        movingCardsUntilCounter = movingCardsUntilNumberOfHits;
        movingCardsUntil = true;
        actMoveTopCardToPlay();
    }
}

void Player::moveOneCardUntil(CardItem *card)
{
    moveTopCardTimer->stop();

    const bool isMatch = card && movingCardsUntilFilter.check(card->getCard().getCardPtr());

    if (isMatch && movingCardsUntilAutoPlay) {
        // Directly calling playCard will deadlock, since we are already in the middle of processing an event.
        // Use QTimer::singleShot to queue up the playCard on the event loop.
        QTimer::singleShot(0, this, [card, this] { playCard(card, false); });
    }

    if (zones.value("deck")->getCards().empty() || !card) {
        stopMoveTopCardsUntil();
    } else if (isMatch) {
        --movingCardsUntilCounter;
        if (movingCardsUntilCounter > 0) {
            moveTopCardTimer->start();
        } else {
            stopMoveTopCardsUntil();
        }
    } else {
        moveTopCardTimer->start();
    }
}

/**
 * @brief Immediately stops any ongoing `play top card to stack until...` process, resetting all variables involved.
 */
void Player::stopMoveTopCardsUntil()
{
    moveTopCardTimer->stop();
    movingCardsUntilCounter = 0;
    movingCardsUntil = false;
}

void Player::actMoveTopCardToBottom()
{
    if (zones.value("deck")->getCards().empty()) {
        return;
    }

    Command_MoveCard cmd;
    cmdSetTopCard(cmd);
    cmd.set_target_zone("deck");
    cmd.set_x(-1); // bottom of deck
    cmd.set_y(0);

    sendGameCommand(cmd);
}

void Player::actMoveTopCardToPlay()
{
    if (zones.value("deck")->getCards().empty()) {
        return;
    }

    Command_MoveCard cmd;
    cmdSetTopCard(cmd);
    cmd.set_target_zone("stack");
    cmd.set_x(-1);
    cmd.set_y(0);

    sendGameCommand(cmd);
}

void Player::actMoveTopCardToPlayFaceDown()
{
    if (zones.value("deck")->getCards().empty()) {
        return;
    }

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
    if (zones.value("deck")->getCards().empty()) {
        return;
    }

    Command_MoveCard cmd;
    cmdSetBottomCard(cmd);
    cmd.set_target_zone("grave");
    cmd.set_x(0);
    cmd.set_y(0);

    sendGameCommand(cmd);
}

void Player::actMoveBottomCardToExile()
{
    if (zones.value("deck")->getCards().empty()) {
        return;
    }

    Command_MoveCard cmd;
    cmdSetBottomCard(cmd);
    cmd.set_target_zone("rfg");
    cmd.set_x(0);
    cmd.set_y(0);

    sendGameCommand(cmd);
}

void Player::actMoveBottomCardsToGrave()
{
    const int maxCards = zones.value("deck")->getCards().size();
    if (maxCards == 0) {
        return;
    }

    bool ok;
    int number =
        QInputDialog::getInt(game, tr("Move bottom cards to grave"), tr("Number of cards: (max. %1)").arg(maxCards),
                             defaultNumberBottomCards, 1, maxCards, 1, &ok);
    if (!ok) {
        return;
    } else if (number > maxCards) {
        number = maxCards;
    }
    defaultNumberBottomCards = number;

    Command_MoveCard cmd;
    cmd.set_start_zone("deck");
    cmd.set_target_player_id(getId());
    cmd.set_target_zone("grave");
    cmd.set_x(0);
    cmd.set_y(0);

    for (int i = maxCards - number; i < maxCards; ++i) {
        cmd.mutable_cards_to_move()->add_card()->set_card_id(i);
    }

    sendGameCommand(cmd);
}

void Player::actMoveBottomCardsToExile()
{
    const int maxCards = zones.value("deck")->getCards().size();
    if (maxCards == 0) {
        return;
    }

    bool ok;
    int number =
        QInputDialog::getInt(game, tr("Move bottom cards to exile"), tr("Number of cards: (max. %1)").arg(maxCards),
                             defaultNumberBottomCards, 1, maxCards, 1, &ok);
    if (!ok) {
        return;
    } else if (number > maxCards) {
        number = maxCards;
    }
    defaultNumberBottomCards = number;

    Command_MoveCard cmd;
    cmd.set_start_zone("deck");
    cmd.set_target_player_id(getId());
    cmd.set_target_zone("rfg");
    cmd.set_x(0);
    cmd.set_y(0);

    for (int i = maxCards - number; i < maxCards; ++i) {
        cmd.mutable_cards_to_move()->add_card()->set_card_id(i);
    }

    sendGameCommand(cmd);
}

void Player::actMoveBottomCardToTop()
{
    if (zones.value("deck")->getCards().empty()) {
        return;
    }

    Command_MoveCard cmd;
    cmdSetBottomCard(cmd);
    cmd.set_target_zone("deck");
    cmd.set_x(0); // top of deck
    cmd.set_y(0);

    sendGameCommand(cmd);
}

/**
 * Selects all cards in the given zone.
 *
 * @param zone The zone to select from
 * @param filter A predicate to filter which cards are selected. Defaults to always returning true.
 */
static void selectCardsInZone(
    const CardZone *zone,
    std::function<bool(const CardItem *)> filter = [](const CardItem *) { return true; })
{
    if (!zone) {
        return;
    }

    for (auto &cardItem : zone->getCards()) {
        if (cardItem && filter(cardItem)) {
            cardItem->setSelected(true);
        }
    }
}

void Player::actSelectAll()
{
    const CardItem *card = game->getActiveCard();
    if (!card) {
        return;
    }

    selectCardsInZone(card->getZone());
}

void Player::actSelectRow()
{
    const CardItem *card = game->getActiveCard();
    if (!card) {
        return;
    }

    auto isSameRow = [card](const CardItem *cardItem) {
        return qAbs(card->scenePos().y() - cardItem->scenePos().y()) < 50;
    };
    selectCardsInZone(card->getZone(), isSameRow);
}

void Player::actSelectColumn()
{
    const CardItem *card = game->getActiveCard();
    if (!card) {
        return;
    }

    auto isSameColumn = [card](const CardItem *cardItem) { return cardItem->x() == card->x(); };
    selectCardsInZone(card->getZone(), isSameColumn);
}

void Player::actDrawBottomCard()
{
    if (zones.value("deck")->getCards().empty()) {
        return;
    }

    Command_MoveCard cmd;
    cmdSetBottomCard(cmd);
    cmd.set_target_zone("hand");
    cmd.set_x(0);
    cmd.set_y(0);

    sendGameCommand(cmd);
}

void Player::actDrawBottomCards()
{
    const int maxCards = zones.value("deck")->getCards().size();
    if (maxCards == 0) {
        return;
    }

    bool ok;
    int number = QInputDialog::getInt(game, tr("Draw bottom cards"), tr("Number of cards: (max. %1)").arg(maxCards),
                                      defaultNumberBottomCards, 1, maxCards, 1, &ok);
    if (!ok) {
        return;
    } else if (number > maxCards) {
        number = maxCards;
    }
    defaultNumberBottomCards = number;

    Command_MoveCard cmd;
    cmd.set_start_zone("deck");
    cmd.set_target_player_id(getId());
    cmd.set_target_zone("hand");
    cmd.set_x(0);
    cmd.set_y(0);

    for (int i = maxCards - number; i < maxCards; ++i) {
        cmd.mutable_cards_to_move()->add_card()->set_card_id(i);
    }

    sendGameCommand(cmd);
}

void Player::actMoveBottomCardToPlay()
{
    if (zones.value("deck")->getCards().empty()) {
        return;
    }

    Command_MoveCard cmd;
    cmdSetBottomCard(cmd);
    cmd.set_target_zone("stack");
    cmd.set_x(-1);
    cmd.set_y(0);

    sendGameCommand(cmd);
}

void Player::actMoveBottomCardToPlayFaceDown()
{
    if (zones.value("deck")->getCards().empty()) {
        return;
    }

    CardZone *zone = zones.value("deck");
    int lastCard = zone->getCards().size() - 1;

    Command_MoveCard cmd;
    cmd.set_start_zone("deck");
    auto *cardToMove = cmd.mutable_cards_to_move()->add_card();
    cardToMove->set_card_id(lastCard);
    cardToMove->set_face_down(true);

    cmd.set_target_player_id(getId());
    cmd.set_target_zone("table");
    cmd.set_x(-1);
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
    DlgRollDice dlg(game);
    if (!dlg.exec()) {
        return;
    }

    Command_RollDie cmd;
    cmd.set_sides(dlg.getDieSideCount());
    cmd.set_count(dlg.getDiceToRollCount());
    sendGameCommand(cmd);
}

void Player::actCreateToken()
{
    DlgCreateToken dlg(predefinedTokens, game);
    if (!dlg.exec()) {
        return;
    }

    lastTokenInfo = dlg.getTokenInfo();

    ExactCard correctedCard = CardDatabaseManager::getInstance()->guessCard({lastTokenInfo.name});
    if (correctedCard) {
        lastTokenInfo.name = correctedCard.getName();
        lastTokenTableRow = TableZone::clampValidTableRow(2 - correctedCard.getInfo().getTableRow());
        if (lastTokenInfo.pt.isEmpty()) {
            lastTokenInfo.pt = correctedCard.getInfo().getPowTough();
        }
    }

    aCreateAnotherToken->setEnabled(true);
    aCreateAnotherToken->setText(tr("C&reate another %1 token").arg(lastTokenInfo.name));
    actCreateAnotherToken();
}

void Player::actCreateAnotherToken()
{
    if (lastTokenInfo.name.isEmpty()) {
        return;
    }

    Command_CreateToken cmd;
    cmd.set_zone("table");
    cmd.set_card_name(lastTokenInfo.name.toStdString());
    cmd.set_color(lastTokenInfo.color.toStdString());
    cmd.set_pt(lastTokenInfo.pt.toStdString());
    cmd.set_annotation(lastTokenInfo.annotation.toStdString());
    cmd.set_destroy_on_zone_change(lastTokenInfo.destroy);
    cmd.set_face_down(lastTokenInfo.faceDown);
    cmd.set_x(-1);
    cmd.set_y(lastTokenTableRow);

    sendGameCommand(cmd);
}

void Player::actCreatePredefinedToken()
{
    auto *action = static_cast<QAction *>(sender());
    CardInfoPtr cardInfo = CardDatabaseManager::getInstance()->getCardInfo(action->text());
    if (!cardInfo) {
        return;
    }

    setLastToken(cardInfo);

    actCreateAnotherToken();
}

void Player::actCreateRelatedCard()
{
    const CardItem *sourceCard = game->getActiveCard();
    if (!sourceCard) {
        return;
    }
    auto *action = static_cast<QAction *>(sender());
    // If there is a better way of passing a CardRelation through a QAction, please add it here.
    auto relatedCards = sourceCard->getCardInfo().getAllRelatedCards();
    CardRelation *cardRelation = relatedCards.at(action->data().toInt());

    /*
     * If we make a token via "Token: TokenName"
     * then let's allow it to be created via "create another token"
     */
    if (createRelatedFromRelation(sourceCard, cardRelation) && cardRelation->getCanCreateAnother()) {
        ExactCard cardInfo =
            CardDatabaseManager::getInstance()->getCard({cardRelation->getName(), sourceCard->getProviderId()});
        setLastToken(cardInfo.getCardPtr());
    }
}

void Player::actCreateAllRelatedCards()
{
    const CardItem *sourceCard = game->getActiveCard();
    if (!sourceCard) {
        return;
    }

    auto relatedCards = sourceCard->getCardInfo().getAllRelatedCards();
    if (relatedCards.isEmpty()) {
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
                        bool persistent = cardRelationAll->getIsPersistent();
                        for (int i = 0; i < cardRelationAll->getDefaultCount(); ++i) {
                            createCard(sourceCard, dbName, CardRelation::DoesNotAttach, persistent);
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
                        bool persistent = cardRelationNotExcluded->getIsPersistent();
                        for (int i = 0; i < cardRelationNotExcluded->getDefaultCount(); ++i) {
                            createCard(sourceCard, dbName, CardRelation::DoesNotAttach, persistent);
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
        CardInfoPtr cardInfo = CardDatabaseManager::getInstance()->getCardInfo(cardRelation->getName());
        setLastToken(cardInfo);
    }
}

bool Player::createRelatedFromRelation(const CardItem *sourceCard, const CardRelation *cardRelation)
{
    if (sourceCard == nullptr || cardRelation == nullptr) {
        return false;
    }
    QString dbName = cardRelation->getName();
    bool persistent = cardRelation->getIsPersistent();
    if (cardRelation->getIsVariable()) {
        bool ok;
        dialogSemaphore = true;
        int count = QInputDialog::getInt(game, tr("Create tokens"), tr("Number:"), cardRelation->getDefaultCount(), 1,
                                         MAX_TOKENS_PER_DIALOG, 1, &ok);
        dialogSemaphore = false;
        if (!ok) {
            return false;
        }
        for (int i = 0; i < count; ++i) {
            createCard(sourceCard, dbName, CardRelation::DoesNotAttach, persistent);
        }
    } else if (cardRelation->getDefaultCount() > 1) {
        for (int i = 0; i < cardRelation->getDefaultCount(); ++i) {
            createCard(sourceCard, dbName, CardRelation::DoesNotAttach, persistent);
        }
    } else {
        auto attachType = cardRelation->getAttachType();

        // move card onto table first if attaching from some other zone
        // we only do this for AttachTo because cross-zone TransformInto is already handled server-side
        if (attachType == CardRelation::AttachTo && sourceCard->getZone()->getName() != "table") {
            playCardToTable(sourceCard, false);
        }

        createCard(sourceCard, dbName, attachType, persistent);
    }
    return true;
}

void Player::createCard(const CardItem *sourceCard,
                        const QString &dbCardName,
                        CardRelation::AttachType attachType,
                        bool persistent)
{
    CardInfoPtr cardInfo = CardDatabaseManager::getInstance()->getCardInfo(dbCardName);

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
    if (SettingsCache::instance().getAnnotateTokens()) {
        cmd.set_annotation(cardInfo->getText().toStdString());
    } else {
        cmd.set_annotation("");
    }
    cmd.set_destroy_on_zone_change(!persistent);
    cmd.set_x(gridPoint.x());
    cmd.set_y(gridPoint.y());

    switch (attachType) {
        case CardRelation::DoesNotAttach:
            cmd.set_target_zone("table");
            break;

        case CardRelation::AttachTo:
            cmd.set_target_zone("table"); // We currently only support creating tokens on the table
            cmd.set_target_card_id(sourceCard->getId());
            cmd.set_target_mode(Command_CreateToken::ATTACH_TO);
            break;

        case CardRelation::TransformInto:
            // allow cards to directly transform on stack
            cmd.set_zone(sourceCard->getZone()->getName() == "stack" ? "stack" : "table");
            // Transform card zone changes are handled server-side
            cmd.set_target_zone(sourceCard->getZone()->getName().toStdString());
            cmd.set_target_card_id(sourceCard->getId());
            cmd.set_target_mode(Command_CreateToken::TRANSFORM_INTO);
            cmd.set_card_provider_id(sourceCard->getProviderId().toStdString());
            break;
    }

    sendGameCommand(cmd);
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
                               bool allCards,
                               EventProcessingOptions options)
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
                bool canAnimate = !options.testFlag(SKIP_TAP_ANIMATION) && !moveCardContext;
                card->setTapped(tapped, canAnimate);
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
    auto &cardList = zone->getCards();
    int absStart = event.start();
    if (absStart < 0) { // negative indexes start from the end
        absStart += cardList.length();
    }

    // close all views that contain shuffled cards
    for (auto *view : zone->getViews()) {
        if (view != nullptr) {
            int length = view->getCards().length();
            // we want to close empty views as well
            if (length == 0 || length > absStart) { // note this assumes views always start at the top of the library
                view->close();
                break;
            }
        } else {
            qWarning() << zone->getName() << "of" << getName() << "holds empty zoneview!";
        }
    }

    // remove revealed card name on top of decks
    if (absStart == 0 && !cardList.isEmpty()) {
        cardList.first()->setCardRef({});
        zone->update();
    }

    emit logShuffle(this, zone, event.start(), event.end());
}

void Player::eventRollDie(const Event_RollDie &event)
{
    if (!event.values().empty()) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        QList<uint> rolls(event.values().begin(), event.values().end());
#else
        QList<uint> rolls;
        for (const auto &value : event.values()) {
            rolls.append(value);
        }
#endif
        std::sort(rolls.begin(), rolls.end());
        emit logRollDie(this, static_cast<int>(event.sides()), rolls);
    } else if (event.value()) {
        // Backwards compatibility for old clients
        emit logRollDie(this, static_cast<int>(event.sides()), {event.value()});
    }
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

    CardRef cardRef = {QString::fromStdString(event.card_name()), QString::fromStdString(event.card_provider_id())};
    CardItem *card = new CardItem(this, nullptr, cardRef, event.card_id());
    // use db PT if not provided in event and not face-down
    if (!QString::fromStdString(event.pt()).isEmpty()) {
        card->setPT(QString::fromStdString(event.pt()));
    } else if (!event.face_down()) {
        ExactCard dbCard = card->getCard();
        if (dbCard) {
            card->setPT(dbCard.getInfo().getPowTough());
        }
    }
    card->setColor(QString::fromStdString(event.color()));
    card->setAnnotation(QString::fromStdString(event.annotation()));
    card->setDestroyOnZoneChange(event.destroy_on_zone_change());
    card->setFaceDown(event.face_down());

    emit logCreateToken(this, card->getName(), card->getPT(), card->getFaceDown());
    zone->addCard(card, true, event.x(), event.y());
}

void Player::eventSetCardAttr(const Event_SetCardAttr &event,
                              const GameEventContext &context,
                              EventProcessingOptions options)
{
    CardZone *zone = zones.value(QString::fromStdString(event.zone_name()), 0);
    if (!zone) {
        return;
    }

    if (!event.has_card_id()) {
        const CardList &cards = zone->getCards();
        for (int i = 0; i < cards.size(); ++i) {
            setCardAttrHelper(context, cards.at(i), event.attribute(), QString::fromStdString(event.attr_value()), true,
                              options);
        }
        if (event.attribute() == AttrTapped) {
            emit logSetTapped(this, nullptr, event.attr_value() == "1");
        }
    } else {
        CardItem *card = zone->getCard(event.card_id());
        if (!card) {
            qWarning() << "Player::eventSetCardAttr: card id=" << event.card_id() << "not found";
            return;
        }
        setCardAttrHelper(context, card, event.attribute(), QString::fromStdString(event.attr_value()), false, options);
    }
}

void Player::eventSetCardCounter(const Event_SetCardCounter &event)
{
    CardZone *zone = zones.value(QString::fromStdString(event.zone_name()), 0);
    if (!zone) {
        return;
    }

    CardItem *card = zone->getCard(event.card_id());
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
    if (!ctr) {
        return;
    }
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
    emit logDumpZone(this, zone, event.number_cards(), event.is_reversed());
}

void Player::eventMoveCard(const Event_MoveCard &event, const GameEventContext &context)
{
    Player *startPlayer = game->getPlayers().value(event.start_player_id());
    if (!startPlayer) {
        return;
    }
    QString startZoneString = QString::fromStdString(event.start_zone());
    CardZone *startZone = startPlayer->getZones().value(startZoneString, 0);
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
    if (card == nullptr) {
        return;
    }
    if (startZone != targetZone) {
        card->deleteCardInfoPopup();
    }
    if (event.has_card_name()) {
        QString name = QString::fromStdString(event.card_name());
        QString providerId =
            event.has_new_card_provider_id() ? QString::fromStdString(event.new_card_provider_id()) : "";
        card->setCardRef({name, providerId});
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
    updateCardMenu(card);

    if (movingCardsUntil && startZoneString == "deck" && targetZone->getName() == "stack") {
        moveOneCardUntil(card);
    }
}

void Player::eventFlipCard(const Event_FlipCard &event)
{
    CardZone *zone = zones.value(QString::fromStdString(event.zone_name()), 0);
    if (!zone) {
        return;
    }
    CardItem *card = zone->getCard(event.card_id());
    if (!card) {
        return;
    }

    if (!event.face_down()) {
        QString cardName = QString::fromStdString(event.card_name());
        QString providerId = QString::fromStdString(event.card_provider_id());
        card->setCardRef({cardName, providerId});
    }

    emit logFlipCard(this, card->getName(), event.face_down());
    card->setFaceDown(event.face_down());
    updateCardMenu(card);
}

void Player::eventDestroyCard(const Event_DestroyCard &event)
{
    CardZone *zone = zones.value(QString::fromStdString(event.zone_name()), 0);
    if (!zone) {
        return;
    }

    CardItem *card = zone->getCard(event.card_id());
    if (!card) {
        return;
    }

    QList<CardItem *> attachedCards = card->getAttachedCards();
    // This list is always empty except for buggy server implementations.
    for (auto &attachedCard : attachedCards) {
        attachedCard->setAttachedTo(nullptr);
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
                targetCard = targetZone->getCard(event.target_card_id());
            }
        }
    }

    CardZone *startZone = getZones().value(QString::fromStdString(event.start_zone()), 0);
    if (!startZone) {
        return;
    }

    CardItem *startCard = startZone->getCard(event.card_id());
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
    updateCardMenu(startCard);
}

void Player::eventDrawCards(const Event_DrawCards &event)
{
    CardZone *_deck = zones.value("deck");
    CardZone *_hand = zones.value("hand");

    const int listSize = event.cards_size();
    if (listSize) {
        for (int i = 0; i < listSize; ++i) {
            const ServerInfo_Card &cardInfo = event.cards(i);
            CardItem *card = _deck->takeCard(0, cardInfo.id());
            QString cardName = QString::fromStdString(cardInfo.name());
            QString providerId = QString::fromStdString(cardInfo.provider_id());
            card->setCardRef({cardName, providerId});
            _hand->addCard(card, false, -1);
        }
    } else {
        const int number = event.number();
        for (int i = 0; i < number; ++i) {
            _hand->addCard(_deck->takeCard(0, -1), false, -1);
        }
    }

    _hand->reorganizeCards();
    _deck->reorganizeCards();
    emit logDrawCards(this, event.number(), _deck->getCards().size() == 0);
}

void Player::eventRevealCards(const Event_RevealCards &event, EventProcessingOptions options)
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
        for (const auto &card : cardList) {
            QString cardName = QString::fromStdString(card->name());
            QString providerId = QString::fromStdString(card->provider_id());
            CardItem *cardItem = zone->getCard(card->id());
            if (!cardItem) {
                continue;
            }
            cardItem->setCardRef({cardName, providerId});
            emit logRevealCards(this, zone, card->id(), cardName, this, true, 1);
        }
    } else {
        bool showZoneView = true;
        QString cardName;
        auto cardId = event.card_id_size() == 0 ? -1 : event.card_id(0);
        if (cardList.size() == 1) {
            cardName = QString::fromStdString(cardList.first()->name());

            // Handle case of revealing top card of library in-place
            if (cardId == 0 && dynamic_cast<PileZone *>(zone)) {
                auto card = zone->getCards().first();
                QString providerId = QString::fromStdString(cardList.first()->provider_id());
                card->setCardRef({cardName, providerId});
                zone->update();
                showZoneView = false;
            }
        }
        if (!options.testFlag(SKIP_REVEAL_WINDOW) && showZoneView && !cardList.isEmpty()) {
            static_cast<GameScene *>(scene())->addRevealedZoneView(this, zone, cardList, event.grant_write_access());
        }

        emit logRevealCards(this, zone, cardId, cardName, otherPlayer, false,
                            event.has_number_of_cards() ? event.number_of_cards() : cardList.size(),
                            event.grant_write_access());
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
    if (event.has_always_look_at_top_card()) {
        zone->setAlwaysRevealTopCard(event.always_look_at_top_card());
        emit logAlwaysLookAtTopCard(this, zone, event.always_look_at_top_card());
    }
}

void Player::processGameEvent(GameEvent::GameEventType type,
                              const GameEvent &event,
                              const GameEventContext &context,
                              EventProcessingOptions options)
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
            eventSetCardAttr(event.GetExtension(Event_SetCardAttr::ext), context, options);
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
            eventRevealCards(event.GetExtension(Event_RevealCards::ext), options);
            break;
        case GameEvent::CHANGE_ZONE_PROPERTIES:
            eventChangeZoneProperties(event.GetExtension(Event_ChangeZoneProperties::ext));
            break;
        default: {
            qWarning() << "unhandled game event" << type;
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
    static QSet<QString> builtinZones{/* PileZones */
                                      "deck", "grave", "rfg", "sb",
                                      /* TableZone */
                                      "table",
                                      /* StackZone */
                                      "stack",
                                      /* HandZone */
                                      "hand"};
    clearCounters();
    clearArrows();

    QMutableMapIterator<QString, CardZone *> zoneIt(zones);
    while (zoneIt.hasNext()) {
        zoneIt.next().value()->clearContents();

        if (!builtinZones.contains(zoneIt.key())) {
            zoneIt.remove();
        }
    }

    // Can be null if we are not the local player!
    if (mCustomZones) {
        mCustomZones->clear();
        mCustomZones->menuAction()->setVisible(false);
    }

    const int zoneListSize = info.zone_list_size();
    for (int i = 0; i < zoneListSize; ++i) {
        const ServerInfo_Zone &zoneInfo = info.zone_list(i);

        QString zoneName = QString::fromStdString(zoneInfo.name());
        CardZone *zone = zones.value(zoneName, 0);
        if (!zone) {
            // Create a new CardZone if it doesn't exist

            if (zoneInfo.with_coords()) {
                // Visibility not currently supported for TableZone
                zone = addZone(new TableZone(this, zoneName, this));
            } else {
                // Zones without coordinats are always treated as non-shufflable
                // PileZones, although supporting alternate hand or stack zones
                // might make sense in some scenarios.
                bool contentsKnown;

                switch (zoneInfo.type()) {
                    case ServerInfo_Zone::PrivateZone:
                        contentsKnown = local || judge || (game->getSpectator() && game->getSpectatorsSeeEverything());
                        break;

                    case ServerInfo_Zone::PublicZone:
                        contentsKnown = true;
                        break;

                    case ServerInfo_Zone::HiddenZone:
                        contentsKnown = false;
                        break;
                }

                zone = addZone(new PileZone(this, zoneName, /* isShufflable */ false, contentsKnown, this));
            }

            // Non-builtin zones are hidden by default and can't be interacted
            // with, except through menus.
            zone->setVisible(false);

            if (mCustomZones) {
                mCustomZones->menuAction()->setVisible(true);
                QAction *aViewZone = mCustomZones->addAction(tr("View custom zone '%1'").arg(zoneName));
                aViewZone->setData(zoneName);
                connect(aViewZone, &QAction::triggered, this,
                        [zoneName, this]() { static_cast<GameScene *>(scene())->toggleZoneView(this, zoneName, -1); });
            }

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
                CardItem *startCard = zone->getCard(cardInfo.id());
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

void Player::playCard(CardItem *card, bool faceDown)
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

    ExactCard exactCard = card->getCard();
    if (!exactCard) {
        return;
    }

    const CardInfo &info = exactCard.getInfo();

    int tableRow = info.getTableRow();
    bool playToStack = SettingsCache::instance().getPlayToStack();
    QString currentZone = card->getZone()->getName();
    if (currentZone == "stack" && tableRow == 3) {
        cmd.set_target_zone("grave");
        cmd.set_x(0);
        cmd.set_y(0);
    } else if (!faceDown &&
               ((!playToStack && tableRow == 3) || ((playToStack && tableRow != 0) && currentZone != "stack"))) {
        cmd.set_target_zone("stack");
        cmd.set_x(-1);
        cmd.set_y(0);
    } else {
        tableRow = faceDown ? 2 : info.getTableRow();
        QPoint gridPoint = QPoint(-1, TableZone::clampValidTableRow(2 - tableRow));
        cardToMove->set_face_down(faceDown);
        if (!faceDown) {
            cardToMove->set_pt(info.getPowTough().toStdString());
        }
        cardToMove->set_tapped(!faceDown && info.getCipt());
        if (tableRow != 3)
            cmd.set_target_zone("table");
        cmd.set_x(gridPoint.x());
        cmd.set_y(gridPoint.y());
    }
    sendGameCommand(cmd);
}

/**
 * Like {@link Player::playCard}, but forces the card to be played to the table zone.
 * Cards with tablerow 3 (the stack) will be played to tablerow 1 (the noncreatures row).
 */
void Player::playCardToTable(const CardItem *card, bool faceDown)
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

    ExactCard exactCard = card->getCard();
    if (!exactCard) {
        return;
    }

    const CardInfo &info = exactCard.getInfo();

    int tableRow = faceDown ? 2 : info.getTableRow();
    // default instant/sorcery cards to the noncreatures row
    if (tableRow > 2) {
        tableRow = 1;
    }

    QPoint gridPoint = QPoint(-1, TableZone::clampValidTableRow(2 - tableRow));
    cardToMove->set_face_down(faceDown);
    if (!faceDown) {
        cardToMove->set_pt(info.getPowTough().toStdString());
    }
    cardToMove->set_tapped(!faceDown && info.getCipt());
    cmd.set_target_zone("table");
    cmd.set_x(gridPoint.x());
    cmd.set_y(gridPoint.y());
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

AbstractCounter *Player::addCounter(const ServerInfo_Counter &counter)
{
    return addCounter(counter.id(), QString::fromStdString(counter.name()),
                      convertColorToQColor(counter.counter_color()), counter.radius(), counter.count());
}

AbstractCounter *Player::addCounter(int counterId, const QString &name, QColor color, int radius, int value)
{
    if (counters.contains(counterId)) {
        return nullptr;
    }

    AbstractCounter *ctr;
    if (name == "life") {
        ctr = playerTarget->addCounter(counterId, name, value);
    } else {
        ctr = new GeneralCounter(this, counterId, name, color, radius, value, true, this, game);
    }
    counters.insert(counterId, ctr);
    if (countersMenu && ctr->getMenu()) {
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

void Player::incrementAllCardCounters()
{
    QList<CardItem *> cardsToUpdate;

    auto selectedItems = scene()->selectedItems();
    if (!selectedItems.isEmpty()) {
        // If cards are selected, only update those
        for (const auto &item : selectedItems) {
            auto *card = static_cast<CardItem *>(item);
            cardsToUpdate.append(card);
        }
    } else {
        // If no cards selected, update all cards on table
        const CardList &tableCards = table->getCards();
        cardsToUpdate = tableCards;
    }

    QList<const ::google::protobuf::Message *> commandList;

    for (const auto *card : cardsToUpdate) {
        const auto &cardCounters = card->getCounters();

        QMapIterator<int, int> counterIterator(cardCounters);
        while (counterIterator.hasNext()) {
            counterIterator.next();
            int counterId = counterIterator.key();
            int currentValue = counterIterator.value();
            if (currentValue >= MAX_COUNTERS_ON_CARD) {
                continue;
            }

            auto cmd = std::make_unique<Command_SetCardCounter>();
            cmd->set_zone(card->getZone()->getName().toStdString());
            cmd->set_card_id(card->getId());
            cmd->set_counter_id(counterId);
            cmd->set_counter_value(currentValue + 1);
            commandList.append(cmd.release());
        }
    }

    if (!commandList.isEmpty()) {
        sendGameCommand(prepareGameCommand(commandList));
    }
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

    CardItem *startCard = startZone->getCard(arrow.start_card_id());
    CardItem *targetCard = nullptr;
    if (targetZone) {
        targetCard = targetZone->getCard(arrow.target_card_id());
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
    const qreal padding = 5;
    qreal ySize = boundingRect().y() + marginTop;

    // Place objects
    for (const auto &counter : counters) {
        AbstractCounter *ctr = counter;

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

    if (judge && !local) {
        Command_Judge base;
        GameCommand *c = base.add_game_command();
        base.set_target_id(id);
        c->GetReflection()->MutableMessage(c, cmd.GetDescriptor()->FindExtensionByName("ext"))->CopyFrom(cmd);
        return game->prepareGameCommand(base);
    } else {
        return game->prepareGameCommand(cmd);
    }
}

PendingCommand *Player::prepareGameCommand(const QList<const ::google::protobuf::Message *> &cmdList)
{
    if (judge && !local) {
        Command_Judge base;
        base.set_target_id(id);
        for (int i = 0; i < cmdList.size(); ++i) {
            GameCommand *c = base.add_game_command();
            c->GetReflection()
                ->MutableMessage(c, cmdList[i]->GetDescriptor()->FindExtensionByName("ext"))
                ->CopyFrom(*cmdList[i]);
            delete cmdList[i];
        }
        return game->prepareGameCommand(base);
    } else {
        return game->prepareGameCommand(cmdList);
    }
}

void Player::sendGameCommand(const google::protobuf::Message &command)
{
    if (judge && !local) {
        Command_Judge base;
        GameCommand *c = base.add_game_command();
        base.set_target_id(id);
        c->GetReflection()->MutableMessage(c, command.GetDescriptor()->FindExtensionByName("ext"))->CopyFrom(command);
        game->sendGameCommand(base, id);
    } else {
        game->sendGameCommand(command, id);
    }
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
    int deckSize = zones.value("deck")->getCards().size() + 1; // add the card to move to the deck
    bool ok;
    int number =
        QInputDialog::getInt(game, tr("Place card X cards from top of library"),
                             tr("Which position should this card be placed:") + "\n" + tr("(max. %1)").arg(deckSize),
                             defaultNumberTopCardsToPlaceBelow, 1, deckSize, 1, &ok);
    number -= 1; // indexes start at 0

    if (!ok) {
        return;
    }

    defaultNumberTopCardsToPlaceBelow = number;

    QList<QGraphicsItem *> sel = scene()->selectedItems();
    if (sel.isEmpty()) {
        return;
    }

    QList<CardItem *> cardList;
    while (!sel.isEmpty()) {
        cardList.append(qgraphicsitem_cast<CardItem *>(sel.takeFirst()));
    }

    QList<const ::google::protobuf::Message *> commandList;
    ListOfCardsToMove idList;
    for (const auto &i : cardList) {
        idList.add_card()->set_card_id(i->getId());
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
        for (const auto &card : cardList) {
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
                        ExactCard ec = card->getCard();
                        if (ec) {
                            cmd->set_pt(ec.getInfo().getPowTough().toStdString());
                        }
                    }
                    commandList.append(cmd);
                    break;
                }
                case cmPeek: {
                    auto *cmd = new Command_RevealCards;
                    cmd->set_zone_name(card->getZone()->getName().toStdString());
                    cmd->add_card_id(card->getId());
                    cmd->set_player_id(id);
                    commandList.append(cmd);
                    break;
                }
                case cmClone: {
                    auto *cmd = new Command_CreateToken;
                    cmd->set_zone("table");
                    cmd->set_card_name(card->getName().toStdString());
                    cmd->set_card_provider_id(card->getProviderId().toStdString());
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
        CardZone *zone = cardList[0]->getZone();
        if (!zone) {
            return;
        }

        Player *startPlayer = zone->getPlayer();
        if (!startPlayer) {
            return;
        }

        int startPlayerId = startPlayer->getId();
        QString startZone = zone->getName();

        ListOfCardsToMove idList;
        for (const auto &i : cardList) {
            idList.add_card()->set_card_id(i->getId());
        }

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

                if (idList.card_size() > 1) {
                    auto *scmd = new Command_Shuffle;
                    scmd->set_zone_name("deck");
                    scmd->set_start(0);
                    scmd->set_end(idList.card_size() - 1); // inclusive, the indexed card at end will be shuffled
                    // Server process events backwards, so...
                    commandList.append(scmd);
                }

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
    int playerid = id;

    QList<const ::google::protobuf::Message *> commandList;
    for (const auto &item : scene()->selectedItems()) {
        auto *card = static_cast<CardItem *>(item);
        QString pt = card->getPT();
        const auto ptList = parsePT(pt);
        QString newpt;
        if (ptList.isEmpty()) {
            newpt = QString::number(deltaP) + (deltaT ? "/" + QString::number(deltaT) : "");
        } else if (ptList.size() == 1) {
            newpt = QString::number(ptList.at(0).toInt() + deltaP) + (deltaT ? "/" + QString::number(deltaT) : "");
        } else {
            newpt =
                QString::number(ptList.at(0).toInt() + deltaP) + "/" + QString::number(ptList.at(1).toInt() + deltaT);
        }

        auto *cmd = new Command_SetCardAttr;
        cmd->set_zone(card->getZone()->getName().toStdString());
        cmd->set_card_id(card->getId());
        cmd->set_attribute(AttrPT);
        cmd->set_attr_value(newpt.toStdString());
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
    for (const auto &item : scene()->selectedItems()) {
        auto *card = static_cast<CardItem *>(item);
        QString ptString;
        if (!card->getFaceDown()) { // leave the pt empty if the card is face down
            ExactCard ec = card->getCard();
            if (ec) {
                ptString = ec.getInfo().getPowTough();
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

    if (!commandList.empty()) {
        game->sendGameCommand(prepareGameCommand(commandList), playerid);
    }
}

QVariantList Player::parsePT(const QString &pt)
{
    QVariantList ptList = QVariantList();
    if (!pt.isEmpty()) {
        int sep = pt.indexOf('/');
        if (sep == 0) {
            ptList.append(QVariant(pt.mid(1))); // cut off starting '/' and take full string
        } else {
            int start = 0;
            for (;;) {
                QString item = pt.mid(start, sep - start);
                if (item.isEmpty()) {
                    ptList.append(QVariant(QString()));
                } else if (item[0] == '+') {
                    ptList.append(QVariant(item.mid(1).toInt())); // add as int
                } else if (item[0] == '-') {
                    ptList.append(QVariant(item.toInt())); // add as int
                } else {
                    ptList.append(QVariant(item)); // add as qstring
                }
                if (sep == -1) {
                    break;
                }
                start = sep + 1;
                sep = pt.indexOf('/', start);
            }
        }
    }
    return ptList;
}

void Player::actSetPT()
{
    QString oldPT;
    int playerid = id;

    auto sel = scene()->selectedItems();
    for (const auto &item : sel) {
        auto *card = static_cast<CardItem *>(item);
        if (!card->getPT().isEmpty()) {
            oldPT = card->getPT();
        }
    }
    bool ok;
    dialogSemaphore = true;
    QString pt =
        getTextWithMax(game, tr("Change power/toughness"), tr("Change stats to:"), QLineEdit::Normal, oldPT, &ok);
    dialogSemaphore = false;
    if (clearCardsToDelete() || !ok) {
        return;
    }

    const auto ptList = parsePT(pt);
    bool empty = ptList.isEmpty();

    QList<const ::google::protobuf::Message *> commandList;
    for (const auto &item : sel) {
        auto *card = static_cast<CardItem *>(item);
        auto *cmd = new Command_SetCardAttr;
        QString newpt = QString();
        if (!empty) {
            const auto oldpt = parsePT(card->getPT());
            int ptIter = 0;
            for (const auto &_item : ptList) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                if (_item.typeId() == QMetaType::Type::Int) {
#else
                if (_item.type() == QVariant::Int) {
#endif
                    int oldItem = ptIter < oldpt.size() ? oldpt.at(ptIter).toInt() : 0;
                    newpt += '/' + QString::number(oldItem + _item.toInt());
                } else {
                    newpt += '/' + _item.toString();
                }
                ++ptIter;
            }
            newpt = newpt.mid(1);
        }

        cmd->set_zone(card->getZone()->getName().toStdString());
        cmd->set_card_id(card->getId());
        cmd->set_attribute(AttrPT);
        cmd->set_attr_value(newpt.toStdString());
        commandList.append(cmd);

        if (local) {
            playerid = card->getZone()->getPlayer()->getId();
        }
    }

    game->sendGameCommand(prepareGameCommand(commandList), playerid);
}

void Player::actDrawArrow()
{
    auto *card = game->getActiveCard();
    if (card) {
        card->drawArrow(Qt::red);
    }
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

void Player::actFlowP()
{
    actIncPT(1, -1);
}

void Player::actFlowT()
{
    actIncPT(-1, 1);
}

void AnnotationDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return && event->modifiers() & Qt::ControlModifier) {
        event->accept();
        accept();
        return;
    }
    QInputDialog::keyPressEvent(event);
}

void Player::actSetAnnotation()
{
    QString oldAnnotation;
    auto sel = scene()->selectedItems();
    for (const auto &item : sel) {
        auto *card = static_cast<CardItem *>(item);
        if (!card->getAnnotation().isEmpty()) {
            oldAnnotation = card->getAnnotation();
        }
    }

    dialogSemaphore = true;
    AnnotationDialog *dialog = new AnnotationDialog(game);
    dialog->setOptions(QInputDialog::UsePlainTextEditForTextInput);
    dialog->setWindowTitle(tr("Set annotation"));
    dialog->setLabelText(tr("Please enter the new annotation:"));
    dialog->setTextValue(oldAnnotation);
    bool ok = dialog->exec();
    dialogSemaphore = false;
    if (clearCardsToDelete() || !ok) {
        return;
    }
    QString annotation = dialog->textValue().left(MAX_NAME_LENGTH);

    QList<const ::google::protobuf::Message *> commandList;
    for (const auto &item : sel) {
        auto *card = static_cast<CardItem *>(item);
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
    auto *card = game->getActiveCard();
    if (!card) {
        return;
    }

    card->drawAttachArrow();
}

void Player::actUnattach()
{
    QList<const ::google::protobuf::Message *> commandList;
    for (QGraphicsItem *item : scene()->selectedItems()) {
        auto *card = static_cast<CardItem *>(item);

        if (!card->getAttachedTo()) {
            continue;
        }

        auto *cmd = new Command_AttachCard;
        cmd->set_start_zone(card->getZone()->getName().toStdString());
        cmd->set_card_id(card->getId());
        commandList.append(cmd);
    }
    sendGameCommand(prepareGameCommand(commandList));
}

void Player::actCardCounterTrigger()
{
    auto *action = static_cast<QAction *>(sender());
    int counterId = action->data().toInt() / 1000;
    QList<const ::google::protobuf::Message *> commandList;
    switch (action->data().toInt() % 1000) {
        case 9: { // increment counter
            for (const auto &item : scene()->selectedItems()) {
                auto *card = static_cast<CardItem *>(item);
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
        case 10: { // decrement counter
            for (const auto &item : scene()->selectedItems()) {
                auto *card = static_cast<CardItem *>(item);
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
        case 11: { // set counter with dialog
            bool ok;
            dialogSemaphore = true;

            int oldValue = 0;
            if (scene()->selectedItems().size() == 1) {
                auto *card = static_cast<CardItem *>(scene()->selectedItems().first());
                oldValue = card->getCounters().value(counterId, 0);
            }
            int number = QInputDialog::getInt(game, tr("Set counters"), tr("Number:"), oldValue, 0,
                                              MAX_COUNTERS_ON_CARD, 1, &ok);
            dialogSemaphore = false;
            if (clearCardsToDelete() || !ok) {
                return;
            }

            for (const auto &item : scene()->selectedItems()) {
                auto *card = static_cast<CardItem *>(item);
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

/**
 * @brief returns true if the zone is a unwritable reveal zone view (eg a card reveal window). Will return false if zone
 * is nullptr.
 */
static bool isUnwritableRevealZone(CardZone *zone)
{
    if (auto *view = qobject_cast<ZoneViewZone *>(zone)) {
        return view->getRevealZone() && !view->getWriteableRevealZone();
    }
    return false;
}

void Player::playSelectedCards(const bool faceDown)
{
    QList<CardItem *> selectedCards;
    for (const auto &item : scene()->selectedItems()) {
        auto *card = static_cast<CardItem *>(item);
        selectedCards.append(card);
    }

    // CardIds will get shuffled downwards when cards leave the deck.
    // We need to iterate through the cards in reverse order so cardIds don't get changed out from under us as we play
    // out the cards one-by-one.
    std::sort(selectedCards.begin(), selectedCards.end(),
              [](const auto &card1, const auto &card2) { return card1->getId() > card2->getId(); });

    for (auto &card : selectedCards) {
        if (card && !isUnwritableRevealZone(card->getZone()) && card->getZone()->getName() != "table") {
            playCard(card, faceDown);
        }
    }
}

void Player::actPlay()
{
    playSelectedCards(false);
}

void Player::actPlayFacedown()
{
    playSelectedCards(true);
}

void Player::actHide()
{
    for (const auto &item : scene()->selectedItems()) {
        auto *card = static_cast<CardItem *>(item);
        if (card && isUnwritableRevealZone(card->getZone())) {
            card->getZone()->removeCard(card);
        }
    }
}

void Player::actReveal(QAction *action)
{
    const int otherPlayerId = action->data().toInt();

    Command_RevealCards cmd;
    if (otherPlayerId != -1) {
        cmd.set_player_id(otherPlayerId);
    }

    QList<QGraphicsItem *> sel = scene()->selectedItems();
    while (!sel.isEmpty()) {
        const auto *card = qgraphicsitem_cast<CardItem *>(sel.takeFirst());
        if (!cmd.has_zone_name()) {
            cmd.set_zone_name(card->getZone()->getName().toStdString());
        }
        cmd.add_card_id(card->getId());
    }

    sendGameCommand(cmd);
}

void Player::refreshShortcuts()
{
    if (shortcutsActive) {
        setShortcutsActive();
    }
}

void Player::updateCardMenu(const CardItem *card)
{
    // If bad card OR is a spectator (as spectators don't need card menus), return
    // only update the menu if the card is actually selected
    if (card == nullptr || (game->isSpectator() && !judge) || game->getActiveCard() != card) {
        return;
    }

    QMenu *cardMenu = card->getCardMenu();
    QMenu *ptMenu = card->getPTMenu();
    QMenu *moveMenu = card->getMoveMenu();

    cardMenu->clear();

    bool revealedCard = false;
    bool writeableCard = getLocalOrJudge();
    if (auto *view = qobject_cast<ZoneViewZone *>(card->getZone())) {
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
        cardMenu->addAction(aClone);
        cardMenu->addSeparator();
        cardMenu->addAction(aSelectAll);
        cardMenu->addAction(aSelectColumn);
        addRelatedCardView(card, cardMenu);
    } else if (writeableCard) {
        bool canModifyCard = judge || card->getOwner() == this;

        if (moveMenu->isEmpty() && canModifyCard) {
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

                if (!canModifyCard) {
                    addRelatedCardView(card, cardMenu);
                    addRelatedCardActions(card, cardMenu);

                    cardMenu->addSeparator();
                    cardMenu->addAction(aDrawArrow);
                    cardMenu->addSeparator();
                    cardMenu->addAction(aClone);
                    cardMenu->addSeparator();
                    cardMenu->addAction(aSelectAll);
                    cardMenu->addAction(aSelectRow);
                    return;
                }

                if (ptMenu->isEmpty()) {
                    ptMenu->addAction(aIncP);
                    ptMenu->addAction(aDecP);
                    ptMenu->addAction(aFlowP);
                    ptMenu->addSeparator();
                    ptMenu->addAction(aIncT);
                    ptMenu->addAction(aDecT);
                    ptMenu->addAction(aFlowT);
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
                cardMenu->addSeparator();
                cardMenu->addAction(aSelectAll);
                cardMenu->addAction(aSelectRow);

                cardMenu->addSeparator();
                mCardCounters->clear();
                for (int i = 0; i < aAddCounter.size(); ++i) {
                    mCardCounters->addSeparator();
                    mCardCounters->addAction(aAddCounter[i]);
                    if (card->getCounters().contains(i)) {
                        mCardCounters->addAction(aRemoveCounter[i]);
                    }
                    mCardCounters->addAction(aSetCounter[i]);
                }
                cardMenu->addSeparator();
                cardMenu->addMenu(mCardCounters);
            } else if (card->getZone()->getName() == "stack") {
                // Card is on the stack
                if (canModifyCard) {
                    cardMenu->addAction(aAttach);
                    cardMenu->addAction(aDrawArrow);
                    cardMenu->addSeparator();
                    cardMenu->addAction(aClone);
                    cardMenu->addMenu(moveMenu);
                    cardMenu->addSeparator();
                    cardMenu->addAction(aSelectAll);
                } else {
                    cardMenu->addAction(aDrawArrow);
                    cardMenu->addSeparator();
                    cardMenu->addAction(aClone);
                    cardMenu->addSeparator();
                    cardMenu->addAction(aSelectAll);
                }

                addRelatedCardView(card, cardMenu);
                addRelatedCardActions(card, cardMenu);
            } else if (card->getZone()->getName() == "rfg" || card->getZone()->getName() == "grave") {
                // Card is in the graveyard or exile
                if (canModifyCard) {
                    cardMenu->addAction(aPlay);
                    cardMenu->addAction(aPlayFacedown);

                    cardMenu->addSeparator();
                    cardMenu->addAction(aClone);
                    cardMenu->addMenu(moveMenu);
                    cardMenu->addSeparator();
                    cardMenu->addAction(aSelectAll);
                    cardMenu->addAction(aSelectColumn);

                    cardMenu->addSeparator();
                    cardMenu->addAction(aAttach);
                    cardMenu->addAction(aDrawArrow);
                } else {
                    cardMenu->addAction(aClone);
                    cardMenu->addSeparator();
                    cardMenu->addAction(aSelectAll);
                    cardMenu->addAction(aSelectColumn);
                    cardMenu->addSeparator();
                    cardMenu->addAction(aDrawArrow);
                }

                addRelatedCardView(card, cardMenu);
                addRelatedCardActions(card, cardMenu);
            } else {
                // Card is in hand or a custom zone specified by server
                cardMenu->addAction(aPlay);
                cardMenu->addAction(aPlayFacedown);

                QMenu *revealMenu = cardMenu->addMenu(tr("Re&veal to..."));
                initContextualPlayersMenu(revealMenu);
                connect(revealMenu, &QMenu::triggered, this, &Player::actReveal);

                cardMenu->addSeparator();
                cardMenu->addAction(aClone);
                cardMenu->addMenu(moveMenu);

                // actions that are really wonky when done from deck or sideboard
                if (card->getZone()->getName() == "hand") {
                    cardMenu->addSeparator();
                    cardMenu->addAction(aAttach);
                    cardMenu->addAction(aDrawArrow);
                }

                cardMenu->addSeparator();
                cardMenu->addAction(aSelectAll);
                if (qobject_cast<ZoneViewZone *>(card->getZone())) {
                    cardMenu->addAction(aSelectColumn);
                }

                addRelatedCardView(card, cardMenu);
                if (card->getZone()->getName() == "hand") {
                    addRelatedCardActions(card, cardMenu);
                }
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
            cardMenu->addSeparator();
            cardMenu->addAction(aSelectAll);
        }
    }
}

void Player::addRelatedCardView(const CardItem *card, QMenu *cardMenu)
{
    if (!card || !cardMenu) {
        return;
    }
    auto exactCard = card->getCard();
    if (!exactCard) {
        return;
    }

    bool atLeastOneGoodRelationFound = false;
    QList<CardRelation *> relatedCards = exactCard.getInfo().getAllRelatedCards();
    for (const CardRelation *cardRelation : relatedCards) {
        CardInfoPtr relatedCard = CardDatabaseManager::getInstance()->getCardInfo(cardRelation->getName());
        if (relatedCard != nullptr) {
            atLeastOneGoodRelationFound = true;
            break;
        }
    }

    if (!atLeastOneGoodRelationFound) {
        return;
    }

    cardMenu->addSeparator();
    auto viewRelatedCards = new QMenu(tr("View related cards"));
    cardMenu->addMenu(viewRelatedCards);
    for (const CardRelation *relatedCard : relatedCards) {
        QString relatedCardName = relatedCard->getName();
        CardRef cardRef = {relatedCardName, exactCard.getPrinting().getUuid()};
        QAction *viewCard = viewRelatedCards->addAction(relatedCardName);
        connect(viewCard, &QAction::triggered, game, [this, cardRef] { game->viewCardInfo(cardRef); });
    }
}

void Player::addRelatedCardActions(const CardItem *card, QMenu *cardMenu)
{
    if (!card || !cardMenu) {
        return;
    }
    auto exactCard = card->getCard();
    if (!exactCard) {
        return;
    }

    QList<CardRelation *> relatedCards = exactCard.getInfo().getAllRelatedCards();
    if (relatedCards.isEmpty()) {
        return;
    }

    cardMenu->addSeparator();
    int index = 0;
    QAction *createRelatedCards = nullptr;
    for (const CardRelation *cardRelation : relatedCards) {
        ExactCard relatedCard =
            CardDatabaseManager::getInstance()->getCard({cardRelation->getName(), exactCard.getPrinting().getUuid()});
        if (!relatedCard) {
            relatedCard = CardDatabaseManager::getInstance()->getCard({cardRelation->getName()});
        }
        if (!relatedCard) {
            continue;
        }

        QString relatedCardName;
        if (relatedCard.getInfo().getPowTough().size() > 0) {
            relatedCardName = relatedCard.getInfo().getPowTough() + " " + relatedCard.getName(); // "n/n name"
        } else {
            relatedCardName = relatedCard.getName(); // "name"
        }

        QString text = tr("Token: ");
        if (cardRelation->getDoesAttach()) {
            text +=
                tr(cardRelation->getDoesTransform() ? "Transform into " : "Attach to ") + "\"" + relatedCardName + "\"";
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
        connect(createRelated, &QAction::triggered, this, &Player::actCreateRelatedCard);
        cardMenu->addAction(createRelated);
    }

    if (createRelatedCards) {
        if (shortcutsActive) {
            createRelatedCards->setShortcuts(
                SettingsCache::instance().shortcuts().getShortcut("Player/aCreateRelatedTokens"));
        }
        connect(createRelatedCards, &QAction::triggered, this, &Player::actCreateAllRelatedCards);
        cardMenu->addAction(createRelatedCards);
    }
}

void Player::setCardMenu(QMenu *menu)
{
    if (aCardMenu != nullptr) {
        aCardMenu->setEnabled(menu != nullptr);
        if (menu) {
            aCardMenu->setMenu(menu);
        }
    }
}

QMenu *Player::getCardMenu() const
{
    if (aCardMenu != nullptr) {
        return aCardMenu->menu();
    } else {
        return nullptr;
    }
}

QString Player::getName() const
{
    return QString::fromStdString(userInfo->name());
}

qreal Player::getMinimumWidth() const
{
    qreal result = table->getMinimumWidth() + CARD_HEIGHT + 15 + counterAreaWidth + stack->boundingRect().width();
    if (!SettingsCache::instance().getHorizontalHand()) {
        result += hand->boundingRect().width();
    }
    return result;
}

void Player::setGameStarted()
{
    if (local) {
        aAlwaysRevealTopCard->setChecked(false);
        aAlwaysLookAtTopCard->setChecked(false);
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

void Player::setZoneId(int _zoneId)
{
    zoneId = _zoneId;
    playerArea->setPlayerZoneId(_zoneId);
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
    if (!SettingsCache::instance().getHorizontalHand()) {
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

    lastTokenInfo = {.name = cardInfo->getName(),
                     .color = cardInfo->getColors().isEmpty() ? QString() : cardInfo->getColors().left(1).toLower(),
                     .pt = cardInfo->getPowTough(),
                     .annotation = SettingsCache::instance().getAnnotateTokens() ? cardInfo->getText() : "",
                     .destroy = true};

    lastTokenTableRow = TableZone::clampValidTableRow(2 - cardInfo->getTableRow());
    aCreateAnotherToken->setText(tr("C&reate another %1 token").arg(lastTokenInfo.name));
    aCreateAnotherToken->setEnabled(true);
}
