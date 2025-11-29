#include "player.h"

#include "../../interface/theme_manager.h"
#include "../../interface/widgets/tabs/tab_game.h"
#include "../board/arrow_item.h"
#include "../board/card_item.h"
#include "../board/card_list.h"
#include "../board/counter_general.h"
#include "../game_scene.h"
#include "../zones/hand_zone.h"
#include "../zones/pile_zone.h"
#include "../zones/stack_zone.h"
#include "../zones/table_zone.h"
#include "player_actions.h"
#include "player_target.h"

#include <QDebug>
#include <QMenu>
#include <QMetaType>
#include <QPainter>
#include <QtConcurrent>
#include <libcockatrice/protocol/pb/command_attach_card.pb.h>
#include <libcockatrice/protocol/pb/command_set_card_counter.pb.h>
#include <libcockatrice/protocol/pb/event_create_arrow.pb.h>
#include <libcockatrice/protocol/pb/event_create_counter.pb.h>
#include <libcockatrice/protocol/pb/event_draw_cards.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_player.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_user.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_zone.pb.h>
#include <libcockatrice/utility/color.h>

Player::Player(const ServerInfo_User &info, int _id, bool _local, bool _judge, AbstractGame *_parent)
    : QObject(_parent), game(_parent), playerInfo(new PlayerInfo(info, _id, _local, _judge)),
      playerEventHandler(new PlayerEventHandler(this)), playerActions(new PlayerActions(this)), active(false),
      conceded(false), deck(nullptr), zoneId(0), dialogSemaphore(false)
{
    initializeZones();

    playerMenu = new PlayerMenu(this);
    graphicsItem = new PlayerGraphicsItem(this);
    playerMenu->setMenusForGraphicItems();

    connect(this, &Player::activeChanged, graphicsItem, &PlayerGraphicsItem::onPlayerActiveChanged);

    connect(this, &Player::openDeckEditor, game->getTab(), &TabGame::openDeckEditor);

    forwardActionSignalsToEventHandler();
}

// Event Handler is the controller i.e. everything hooks up to this to know about player state
// Player should forward (private) signals to the event handler

void Player::forwardActionSignalsToEventHandler()
{
    connect(playerActions, &PlayerActions::logSetTapped, playerEventHandler, &PlayerEventHandler::logSetTapped);
    connect(playerActions, &PlayerActions::logSetDoesntUntap, playerEventHandler,
            &PlayerEventHandler::logSetDoesntUntap);
    connect(playerActions, &PlayerActions::logSetAnnotation, playerEventHandler, &PlayerEventHandler::logSetAnnotation);
    connect(playerActions, &PlayerActions::logSetPT, playerEventHandler, &PlayerEventHandler::logSetPT);
}

void Player::initializeZones()
{
    addZone(new PileZoneLogic(this, "deck", false, true, false, this));
    addZone(new PileZoneLogic(this, "grave", false, false, true, this));
    addZone(new PileZoneLogic(this, "rfg", false, false, true, this));
    addZone(new PileZoneLogic(this, "sb", false, false, false, this));
    addZone(new TableZoneLogic(this, "table", true, false, true, this));
    addZone(new StackZoneLogic(this, "stack", true, false, true, this));
    bool visibleHand = playerInfo->getLocalOrJudge() ||
                       (game->getPlayerManager()->isSpectator() && game->getGameMetaInfo()->spectatorsOmniscient());
    addZone(new HandZoneLogic(this, "hand", false, false, visibleHand, this));
}

Player::~Player()
{
    qCInfo(PlayerLog) << "Player destructor:" << getPlayerInfo()->getName();

    QMapIterator<QString, CardZoneLogic *> i(zones);
    while (i.hasNext())
        delete i.next().value();
    zones.clear();

    delete playerMenu;
    delete getPlayerInfo()->userInfo;
}

void Player::clear()
{
    clearArrows();

    QMapIterator<QString, CardZoneLogic *> i(zones);
    while (i.hasNext()) {
        i.next().value()->clearContents();
    }

    clearCounters();
}

void Player::setConceded(bool _conceded)
{
    if (conceded != _conceded) {
        conceded = _conceded;

        getGraphicsItem()->setVisible(!conceded);
        if (conceded) {
            clear();
        }
        emit concededChanged(getPlayerInfo()->getId(), conceded);
    }
}

void Player::setZoneId(int _zoneId)
{
    zoneId = _zoneId;
    graphicsItem->getPlayerArea()->setPlayerZoneId(zoneId);
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

    QMutableMapIterator<QString, CardZoneLogic *> zoneIt(zones);
    while (zoneIt.hasNext()) {
        zoneIt.next().value()->clearContents();

        if (!builtinZones.contains(zoneIt.key())) {
            zoneIt.remove();
        }
    }

    emit clearCustomZonesMenu();

    const int zoneListSize = info.zone_list_size();
    for (int i = 0; i < zoneListSize; ++i) {
        const ServerInfo_Zone &zoneInfo = info.zone_list(i);

        QString zoneName = QString::fromStdString(zoneInfo.name());
        CardZoneLogic *zone = zones.value(zoneName, 0);
        if (!zone) {
            // Create a new CardZone if it doesn't exist

            if (zoneInfo.with_coords()) {
                // Visibility not currently supported for TableZone
                zone = addZone(new TableZoneLogic(this, zoneName, true, false, true, this));
            } else {
                // Zones without coordinats are always treated as non-shufflable
                // PileZones, although supporting alternate hand or stack zones
                // might make sense in some scenarios.
                bool contentsKnown;

                switch (zoneInfo.type()) {
                    case ServerInfo_Zone::PrivateZone:
                        contentsKnown =
                            playerInfo->getLocalOrJudge() || (game->getPlayerManager()->isSpectator() &&
                                                              game->getGameMetaInfo()->spectatorsOmniscient());
                        break;

                    case ServerInfo_Zone::PublicZone:
                        contentsKnown = true;
                        break;

                    case ServerInfo_Zone::HiddenZone:
                        contentsKnown = false;
                        break;
                }

                zone = addZone(new PileZoneLogic(this, zoneName, false, /* isShufflable */ false, contentsKnown, this));
            }

            // Non-builtin zones are hidden by default and can't be interacted
            // with, except through menus.
            emit zone->setGraphicsVisibility(false);

            emit addViewCustomZoneActionToCustomZoneMenu(zoneName);

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
                auto *card = new CardItem(this);
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
        CardZoneLogic *zone = zones.value(QString::fromStdString(zoneInfo.name()), 0);
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

// TODO: Does a player need a DeckLoader?
void Player::setDeck(DeckLoader &_deck)
{
    deck = new DeckLoader(this, _deck.getDeckList());

    emit deckChanged();
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
        ctr = getGraphicsItem()->getPlayerTarget()->addCounter(counterId, name, value);
    } else {
        ctr = new GeneralCounter(this, counterId, name, color, radius, value, true, graphicsItem);
    }
    counters.insert(counterId, ctr);

    if (playerMenu->getCountersMenu() && ctr->getMenu()) {
        playerMenu->getCountersMenu()->addMenu(ctr->getMenu());
    }
    if (playerMenu->getShortcutsActive()) {
        ctr->setShortcutsActive();
    }
    emit rearrangeCounters();
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
    emit rearrangeCounters();
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

    auto selectedItems = getGameScene()->selectedItems();
    if (!selectedItems.isEmpty()) {
        // If cards are selected, only update those
        for (const auto &item : selectedItems) {
            auto *card = static_cast<CardItem *>(item);
            cardsToUpdate.append(card);
        }
    } else {
        // If no cards selected, update all cards on table
        const CardList &tableCards = getTableZone()->getCards();
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
        playerActions->sendGameCommand(playerActions->prepareGameCommand(commandList));
    }
}

ArrowItem *Player::addArrow(const ServerInfo_Arrow &arrow)
{
    const QMap<int, Player *> &playerList = game->getPlayerManager()->getPlayers();
    Player *startPlayer = playerList.value(arrow.start_player_id(), 0);
    Player *targetPlayer = playerList.value(arrow.target_player_id(), 0);
    if (!startPlayer || !targetPlayer) {
        return nullptr;
    }

    CardZoneLogic *startZone = startPlayer->getZones().value(QString::fromStdString(arrow.start_zone()), 0);
    CardZoneLogic *targetZone = nullptr;
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
        return addArrow(arrow.id(), startCard, targetPlayer->getGraphicsItem()->getPlayerTarget(),
                        convertColorToQColor(arrow.arrow_color()));
    }
}

ArrowItem *Player::addArrow(int arrowId, CardItem *startCard, ArrowTarget *targetItem, const QColor &color)
{
    auto *arrow = new ArrowItem(this, arrowId, startCard, targetItem, color);
    arrows.insert(arrowId, arrow);

    getGameScene()->addItem(arrow);
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

void Player::setActive(bool _active)
{
    active = _active;
    emit activeChanged(active);
}

void Player::updateZones()
{
    getTableZone()->reorganizeCards();
}

PlayerGraphicsItem *Player::getGraphicsItem()
{
    return graphicsItem;
}

GameScene *Player::getGameScene()
{
    return getGraphicsItem()->getGameScene();
}

void Player::setGameStarted()
{
    if (playerInfo->local) {
        emit resetTopCardMenuActions();
    }
    setConceded(false);
}
