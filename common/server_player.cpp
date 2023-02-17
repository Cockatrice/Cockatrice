#include "server_player.h"

#include "color.h"
#include "decklist.h"
#include "get_pb_extension.h"
#include "pb/command_attach_card.pb.h"
#include "pb/command_change_zone_properties.pb.h"
#include "pb/command_concede.pb.h"
#include "pb/command_create_arrow.pb.h"
#include "pb/command_create_counter.pb.h"
#include "pb/command_create_token.pb.h"
#include "pb/command_deck_select.pb.h"
#include "pb/command_del_counter.pb.h"
#include "pb/command_delete_arrow.pb.h"
#include "pb/command_draw_cards.pb.h"
#include "pb/command_dump_zone.pb.h"
#include "pb/command_flip_card.pb.h"
#include "pb/command_game_say.pb.h"
#include "pb/command_inc_card_counter.pb.h"
#include "pb/command_inc_counter.pb.h"
#include "pb/command_kick_from_game.pb.h"
#include "pb/command_leave_game.pb.h"
#include "pb/command_move_card.pb.h"
#include "pb/command_mulligan.pb.h"
#include "pb/command_next_turn.pb.h"
#include "pb/command_ready_start.pb.h"
#include "pb/command_reveal_cards.pb.h"
#include "pb/command_reverse_turn.pb.h"
#include "pb/command_roll_die.pb.h"
#include "pb/command_set_active_phase.pb.h"
#include "pb/command_set_card_attr.pb.h"
#include "pb/command_set_card_counter.pb.h"
#include "pb/command_set_counter.pb.h"
#include "pb/command_set_sideboard_lock.pb.h"
#include "pb/command_set_sideboard_plan.pb.h"
#include "pb/command_shuffle.pb.h"
#include "pb/command_undo_draw.pb.h"
#include "pb/context_concede.pb.h"
#include "pb/context_connection_state_changed.pb.h"
#include "pb/context_deck_select.pb.h"
#include "pb/context_move_card.pb.h"
#include "pb/context_mulligan.pb.h"
#include "pb/context_ready_start.pb.h"
#include "pb/context_set_sideboard_lock.pb.h"
#include "pb/context_undo_draw.pb.h"
#include "pb/event_attach_card.pb.h"
#include "pb/event_attach_zone.pb.h"
#include "pb/event_change_zone_properties.pb.h"
#include "pb/event_create_arrow.pb.h"
#include "pb/event_create_counter.pb.h"
#include "pb/event_create_token.pb.h"
#include "pb/event_create_zone.pb.h"
#include "pb/event_del_counter.pb.h"
#include "pb/event_delete_arrow.pb.h"
#include "pb/event_destroy_card.pb.h"
#include "pb/event_destroy_zone.pb.h"
#include "pb/event_draw_cards.pb.h"
#include "pb/event_dump_zone.pb.h"
#include "pb/event_flip_card.pb.h"
#include "pb/event_game_say.pb.h"
#include "pb/event_move_card.pb.h"
#include "pb/event_player_properties_changed.pb.h"
#include "pb/event_reveal_cards.pb.h"
#include "pb/event_reverse_turn.pb.h"
#include "pb/event_roll_die.pb.h"
#include "pb/event_set_card_attr.pb.h"
#include "pb/event_set_card_counter.pb.h"
#include "pb/event_set_counter.pb.h"
#include "pb/event_shuffle.pb.h"
#include "pb/response.pb.h"
#include "pb/response_deck_download.pb.h"
#include "pb/response_dump_zone.pb.h"
#include "pb/serverinfo_player.pb.h"
#include "pb/serverinfo_user.pb.h"
#include "pb/zone_config.pb.h"
#include "rng_abstract.h"
#include "server.h"
#include "server_abstractuserinterface.h"
#include "server_arrow.h"
#include "server_card.h"
#include "server_cardzone.h"
#include "server_counter.h"
#include "server_database_interface.h"
#include "server_game.h"
#include "server_room.h"
#include "stringsizes.h"

#include <QDebug>
#include <QMultiMap>
#include <algorithm>

Server_Player::Server_Player(Server_Game *_game,
                             int _playerId,
                             const ServerInfo_User &_userInfo,
                             bool _spectator,
                             bool _judge,
                             Server_AbstractUserInterface *_userInterface)
    : ServerInfo_User_Container(_userInfo), game(_game), userInterface(_userInterface), deck(nullptr), pingTime(0),
      playerId(_playerId), spectator(_spectator), judge(_judge), nextCardId(0), readyStart(false), conceded(false),
      sideboardLocked(true)
{
}

Server_Player::~Server_Player() = default;

void Server_Player::prepareDestroy()
{
    delete deck;
    deck = nullptr;

    playerMutex.lock();
    if (userInterface) {
        userInterface->playerRemovedFromGame(game);
    }
    playerMutex.unlock();

    clearZones();

    deleteLater();
}

int Server_Player::newCardId()
{
    return nextCardId++;
}

int Server_Player::newCounterId() const
{
    int id = 0;
    QMapIterator<int, Server_Counter *> i(counters);
    while (i.hasNext()) {
        Server_Counter *c = i.next().value();
        if (c->getId() > id) {
            id = c->getId();
        }
    }
    return id + 1;
}

int Server_Player::newArrowId() const
{
    int id = 0;
    for (Server_Arrow *a : arrows) {
        if (a->getId() > id) {
            id = a->getId();
        }
    }
    return id + 1;
}

void Server_Player::setupZones()
{
    // This may need to be customized according to the game rules.
    // ------------------------------------------------------------------

    // Create zones
    auto deckZone = Server_CardZone::create(this, "deck", ZoneType::HiddenZone, Server_CardZone::CanShuffle);
    addZone(deckZone);
    auto sbZone = Server_CardZone::create(this, "sb", ZoneType::HiddenZone);
    addZone(sbZone);
    addZone(Server_CardZone::create(this, "table", ZoneType::PublicZone, Server_CardZone::HasCoords));
    addZone(Server_CardZone::create(this, "hand", ZoneType::PrivateZone));
    addZone(Server_CardZone::create(this, "stack", ZoneType::PublicZone));
    addZone(Server_CardZone::create(this, "grave", ZoneType::PublicZone));
    addZone(Server_CardZone::create(this, "rfg", ZoneType::PublicZone));

    addCounter(new Server_Counter(0, "life", makeColor(255, 255, 255), 25, 20));
    addCounter(new Server_Counter(1, "w", makeColor(255, 255, 150), 20, 0));
    addCounter(new Server_Counter(2, "u", makeColor(150, 150, 255), 20, 0));
    addCounter(new Server_Counter(3, "b", makeColor(150, 150, 150), 20, 0));
    addCounter(new Server_Counter(4, "r", makeColor(250, 150, 150), 20, 0));
    addCounter(new Server_Counter(5, "g", makeColor(150, 255, 150), 20, 0));
    addCounter(new Server_Counter(6, "x", makeColor(255, 255, 255), 20, 0));
    addCounter(new Server_Counter(7, "storm", makeColor(255, 150, 30), 20, 0));

    // ------------------------------------------------------------------

    // Assign card ids and create deck from deck list
    InnerDecklistNode *listRoot = deck->getRoot();
    nextCardId = 0;
    for (int i = 0; i < listRoot->size(); ++i) {
        auto *currentZone = dynamic_cast<InnerDecklistNode *>(listRoot->at(i));
        std::shared_ptr<Server_CardZone> z;
        if (currentZone->getName() == DECK_ZONE_MAIN) {
            z = deckZone;
        } else if (currentZone->getName() == DECK_ZONE_SIDE) {
            z = sbZone;
        } else {
            continue;
        }

        for (int j = 0; j < currentZone->size(); ++j) {
            auto *currentCard = dynamic_cast<DecklistCardNode *>(currentZone->at(j));
            if (!currentCard) {
                continue;
            }
            for (int k = 0; k < currentCard->getNumber(); ++k) {
                z->insertCard(new Server_Card(currentCard->getName(), nextCardId++, 0, 0, z), -1, 0);
            }
        }
    }

    const QList<MoveCard_ToZone> &sideboardPlan = deck->getCurrentSideboardPlan();
    for (const auto &m : sideboardPlan) {
        const QString startZone = nameFromStdString(m.start_zone());
        const QString targetZone = nameFromStdString(m.target_zone());

        std::shared_ptr<Server_CardZone> start, target;
        if (startZone == DECK_ZONE_MAIN) {
            start = deckZone;
        } else if (startZone == DECK_ZONE_SIDE) {
            start = sbZone;
        } else {
            continue;
        }
        if (targetZone == DECK_ZONE_MAIN) {
            target = deckZone;
        } else if (targetZone == DECK_ZONE_SIDE) {
            target = sbZone;
        } else {
            continue;
        }

        for (int j = 0; j < start->getCards().size(); ++j) {
            if (start->getCards()[j]->getName() == nameFromStdString(m.card_name())) {
                Server_Card *card = start->getCard(j, nullptr, true);
                target->insertCard(card, -1, 0);
                break;
            }
        }
    }

    deckZone->shuffle();
}

void Server_Player::clearZones()
{
    zones.clear();

    for (Server_Counter *counter : counters) {
        delete counter;
    }
    counters.clear();

    for (Server_Arrow *arrow : arrows) {
        delete arrow;
    }
    arrows.clear();

    lastDrawList.clear();
}

void Server_Player::getProperties(ServerInfo_PlayerProperties &result, bool withUserInfo)
{
    result.set_player_id(playerId);
    if (withUserInfo) {
        copyUserInfo(*(result.mutable_user_info()), true);
    }
    result.set_spectator(spectator);
    if (!spectator) {
        result.set_conceded(conceded);
        result.set_sideboard_locked(sideboardLocked);
        result.set_ready_start(readyStart);
    }
    result.set_judge(judge);
    if (deck) {
        result.set_deck_hash(deck->getDeckHash().toStdString());
    }
    result.set_ping_seconds(pingTime);
}

void Server_Player::addZone(const std::shared_ptr<Server_CardZone> &zone)
{
    zones.insert(zone->getName(), zone);
}

void Server_Player::destroyZone(QString zoneName)
{
    zones.remove(zoneName);
}

void Server_Player::addArrow(Server_Arrow *arrow)
{
    arrows.insert(arrow->getId(), arrow);
}

bool Server_Player::deleteArrow(int arrowId)
{
    Server_Arrow *arrow = arrows.value(arrowId, 0);
    if (!arrow) {
        return false;
    }
    arrows.remove(arrowId);
    delete arrow;
    return true;
}

void Server_Player::addCounter(Server_Counter *counter)
{
    counters.insert(counter->getId(), counter);
}

Response::ResponseCode Server_Player::drawCards(GameEventStorage &ges, int number)
{
    auto deckZone = zones.value("deck");
    auto handZone = zones.value("hand");
    if (deckZone->getCards().size() < number) {
        number = deckZone->getCards().size();
    }

    Event_DrawCards eventOthers;
    eventOthers.set_number(number);
    Event_DrawCards eventPrivate(eventOthers);

    for (int i = 0; i < number; ++i) {
        Server_Card *card = deckZone->getCard(0, nullptr, true);
        handZone->insertCard(card, -1, 0);
        lastDrawList.append(card->getId());

        ServerInfo_Card *cardInfo = eventPrivate.add_cards();
        cardInfo->set_id(card->getId());
        cardInfo->set_name(card->getName().toStdString());
    }

    ges.enqueueGameEvent(eventPrivate, playerId, GameEventStorageItem::SendToPrivate, playerId);
    ges.enqueueGameEvent(eventOthers, playerId, GameEventStorageItem::SendToOthers);

    if (number > 0) {
        revealTopCardIfNeeded(deckZone, ges);
        int currentKnownCards = deckZone->getCardsBeingLookedAt();
        deckZone->setCardsBeingLookedAt(currentKnownCards - number);
    }

    return Response::RespOk;
}

void Server_Player::revealTopCardIfNeeded(const std::shared_ptr<Server_CardZone> &zone, GameEventStorage &ges)
{
    if (zone->getCards().isEmpty()) {
        return;
    }
    if (zone->getAlwaysRevealTopCard()) {
        Event_RevealCards revealEvent;
        revealEvent.set_zone_name(zone->getName().toStdString());
        revealEvent.add_card_id(0);
        zone->getCards().first()->getInfo(revealEvent.add_cards());

        ges.enqueueGameEvent(revealEvent, playerId);
        return;
    }
    if (zone->getAlwaysLookAtTopCard()) {
        Event_DumpZone dumpEvent;
        dumpEvent.set_zone_owner_id(playerId);
        dumpEvent.set_zone_name(zone->getName().toStdString());
        dumpEvent.set_number_cards(1);
        ges.enqueueGameEvent(dumpEvent, playerId, GameEventStorageItem::SendToOthers);

        Event_RevealCards revealEvent;
        revealEvent.set_zone_name(zone->getName().toStdString());
        revealEvent.set_number_of_cards(1);
        revealEvent.add_card_id(0);
        zone->getCards().first()->getInfo(revealEvent.add_cards());
        ges.enqueueGameEvent(revealEvent, playerId, GameEventStorageItem::SendToPrivate, playerId);
    }
}

static Event_MoveCard makeMoveCardEvent(std::shared_ptr<Server_CardZone> sourceZone,
                                        int sourceId,
                                        int sourcePosition,
                                        bool sourceKnown,
                                        Server_Card *card,
                                        bool targetKnown,
                                        int targetX,
                                        int targetY)
{
    auto targetZone = card->getZone();

    Event_MoveCard event;
    event.set_start_player_id(sourceZone->getPlayer()->getPlayerId());
    event.set_start_zone(sourceZone->getName().toStdString());
    event.set_target_player_id(targetZone->getPlayer()->getPlayerId());
    event.set_target_zone(targetZone->getName().toStdString());
    event.set_x(targetX);
    event.set_y(targetY);
    event.set_face_down(card->getFaceDown());

    if (sourceZone->getType() == ZoneType::PublicZone || sourceKnown) {
        event.set_card_id(sourceId);
    }

    if (sourceZone->getType() == ZoneType::HiddenZone || !sourceKnown) {
        event.set_position(sourcePosition);
    }

    if (targetZone->getType() == ZoneType::PublicZone || targetKnown) {
        event.set_new_card_id(card->getId());
    }

    if (sourceKnown || targetKnown) {
        event.set_card_name(card->getName().toStdString());
    }

    return event;
}

void Server_Player::moveCards(GameEventStorage &ges,
                              const QList<Server_Card *> &cards,
                              std::shared_ptr<Server_CardZone> targetZone,
                              int targetX,
                              int targetY,
                              const QMap<int, CardAttributes> &cardsAttributes)
{
    if (!targetZone->hasCoords()) {
        targetY = 0;

        if (targetX < 0) {
            targetX = targetZone->getCards().size();
        }
    }

    if (targetY < 0)
        targetY = 0;

    Server_Player *targetPlayer = targetZone->getPlayer();

    QList<std::shared_ptr<Server_CardZone>> zonesToReveal;
    for (auto *card : cards) {
        const CardAttributes cardAttributes = cardsAttributes.value(card->getId());
        bool faceDown = cardAttributes.faceDown.value_or(card->getFaceDown());

        auto sourceZone = card->getZone();
        Server_Player *sourcePlayer = sourceZone->getPlayer();

        bool controllerChanges = targetPlayer != sourcePlayer;
        bool zoneChanges = targetZone->getName() != sourceZone->getName();

        int sourcePosition;
        sourceZone->getCard(card->getId(), &sourcePosition);

        if (zoneChanges && card->getDestroyOnZoneChange()) {
            destroyCard(ges, card, targetZone, targetX, targetY);
        } else {
            if (zoneChanges || controllerChanges) {
                game->removeArrowsRelatedToCard(ges, card);
            }

            if (zoneChanges) {
                game->unattachCardsRelatedToCard(ges, card);
            }

            // Remove the card from its zone on the server and determine who knew its name
            int sourceId = card->getId();
            bool sourceBeingLookedAt;
            sourceZone->removeCard(card, sourceBeingLookedAt);

            bool sourceKnownToPlayer =
                sourceZone->getType() == ZoneType::HiddenZone ? sourceBeingLookedAt : !card->getFaceDown();
            bool sourceKnownToOthers = sourceZone->getType() == ZoneType::PublicZone && !card->getFaceDown();

            if (targetZone->hasCoords()) {
                targetX = targetZone->getFreeGridColumn(targetX, targetY, card->getName(), faceDown);
            } else {
                card->resetState();
            }
            targetZone->insertCard(card, targetX, targetY, sourceKnownToPlayer && !controllerChanges);

            card->setFaceDown(faceDown);
            if (controllerChanges || (zoneChanges && faceDown)) {
                card->setId(targetPlayer->newCardId());
            }

            bool targetKnownToOthers = targetZone->getType() == ZoneType::PublicZone && !faceDown;
            bool targetKnownToPlayer =
                targetKnownToOthers || (targetZone->getType() == ZoneType::PrivateZone && !controllerChanges);

            // Our job is done, we just need to emit the relevant events
            Event_MoveCard privateEvent = makeMoveCardEvent(sourceZone, sourceId, sourcePosition, sourceKnownToPlayer,
                                                            card, targetKnownToPlayer, targetX, targetY);
            Event_MoveCard othersEvent = makeMoveCardEvent(sourceZone, sourceId, sourcePosition, sourceKnownToOthers,
                                                           card, targetKnownToOthers, targetY, targetY);

            ges.enqueueGameEvent(privateEvent, sourcePlayer->getPlayerId(), GameEventStorageItem::SendToPrivate,
                                 sourcePlayer->getPlayerId());
            ges.enqueueGameEvent(othersEvent, sourcePlayer->getPlayerId(), GameEventStorageItem::SendToOthers);

            if (targetX == 0) {
                zonesToReveal.append(targetZone);
            }

            targetX++;
        }

        if (sourcePosition == 0) {
            zonesToReveal.append(sourceZone);
        }
    }

    for (const auto &zoneToReveal : zonesToReveal) {
        revealTopCardIfNeeded(zoneToReveal, ges);
    }
}

void Server_Player::unattachCard(GameEventStorage &ges, Server_Card *card)
{
    std::shared_ptr<Server_CardZone> zone = card->getZone();
    Server_Card *parentCard = card->getParentCard();
    card->setParentCard(nullptr);

    Event_AttachCard event;
    event.set_start_zone(zone->getName().toStdString());
    event.set_card_id(card->getId());
    ges.enqueueGameEvent(event, playerId);

    moveCards(ges, {card}, zone, -1, card->getY());
    if (card->getFaceDown()) {
        zone->fixFreeSpaces(ges);
    }

    if (parentCard->getZone()) {
        parentCard->getZone()->updateCardCoordinates(parentCard, parentCard->getX(), parentCard->getY());
    }
}

Response::ResponseCode Server_Player::setCardAttrHelper(GameEventStorage &ges,
                                                        int targetPlayerId,
                                                        const QString &zoneName,
                                                        int cardId,
                                                        CardAttribute attribute,
                                                        const QString &attrValue)
{
    auto zone = getZones().value(zoneName);
    if (!zone) {
        return Response::RespNameNotFound;
    }
    if (!zone->hasCoords()) {
        return Response::RespContextError;
    }

    QString result;
    if (cardId == -1) {
        QListIterator<Server_Card *> CardIterator(zone->getCards());
        while (CardIterator.hasNext()) {
            result = CardIterator.next()->setAttribute(attribute, attrValue, true);
            if (result.isNull()) {
                return Response::RespInvalidCommand;
            }
        }
    } else {
        Server_Card *card = zone->getCard(cardId);
        if (!card) {
            return Response::RespNameNotFound;
        }
        result = card->setAttribute(attribute, attrValue, false);
        if (result.isNull()) {
            return Response::RespInvalidCommand;
        }
    }

    Event_SetCardAttr event;
    event.set_zone_name(zone->getName().toStdString());
    if (cardId != -1) {
        event.set_card_id(cardId);
    }
    event.set_attribute(attribute);
    event.set_attr_value(result.toStdString());
    ges.enqueueGameEvent(event, targetPlayerId);

    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdLeaveGame(const Command_LeaveGame & /*cmd*/, ResponseContainer & /*rc*/, GameEventStorage & /*ges*/)
{
    game->removePlayer(this, Event_Leave::USER_LEFT);
    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdKickFromGame(const Command_KickFromGame &cmd, ResponseContainer & /*rc*/, GameEventStorage & /*ges*/)
{
    if ((game->getHostId() != playerId) && !(userInfo->user_level() & ServerInfo_User::IsModerator)) {
        return Response::RespFunctionNotAllowed;
    }

    if (!game->kickPlayer(cmd.player_id())) {
        return Response::RespNameNotFound;
    }

    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdDeckSelect(const Command_DeckSelect &cmd, ResponseContainer &rc, GameEventStorage &ges)
{
    if (spectator) {
        return Response::RespFunctionNotAllowed;
    }

    DeckList *newDeck;
    if (cmd.has_deck_id()) {
        try {
            newDeck = game->getRoom()->getServer()->getDatabaseInterface()->getDeckFromDatabase(cmd.deck_id(),
                                                                                                userInfo->id());
        } catch (Response::ResponseCode &r) {
            return r;
        }
    } else {
        newDeck = new DeckList(fileFromStdString(cmd.deck()));
    }

    if (!newDeck) {
        return Response::RespInternalError;
    }

    delete deck;
    deck = newDeck;
    sideboardLocked = true;

    Event_PlayerPropertiesChanged event;
    event.mutable_player_properties()->set_sideboard_locked(true);
    event.mutable_player_properties()->set_deck_hash(deck->getDeckHash().toStdString());
    ges.enqueueGameEvent(event, playerId);

    Context_DeckSelect context;
    context.set_deck_hash(deck->getDeckHash().toStdString());
    context.set_sideboard_size(deck->getSideboardSize());
    ges.setGameEventContext(context);

    auto *re = new Response_DeckDownload;
    re->set_deck(deck->writeToString_Native().toStdString());

    rc.setResponseExtension(re);
    return Response::RespOk;
}

Response::ResponseCode Server_Player::cmdSetSideboardPlan(const Command_SetSideboardPlan &cmd,
                                                          ResponseContainer & /*rc*/,
                                                          GameEventStorage & /*ges*/)
{
    if (spectator) {
        return Response::RespFunctionNotAllowed;
    }
    if (readyStart) {
        return Response::RespContextError;
    }
    if (!deck) {
        return Response::RespContextError;
    }
    if (sideboardLocked) {
        return Response::RespContextError;
    }

    QList<MoveCard_ToZone> sideboardPlan;
    for (int i = 0; i < cmd.move_list_size(); ++i) {
        sideboardPlan.append(cmd.move_list(i));
    }
    deck->setCurrentSideboardPlan(sideboardPlan);

    return Response::RespOk;
}

Response::ResponseCode Server_Player::cmdSetSideboardLock(const Command_SetSideboardLock &cmd,
                                                          ResponseContainer & /*rc*/,
                                                          GameEventStorage &ges)
{
    if (spectator) {
        return Response::RespFunctionNotAllowed;
    }
    if (readyStart) {
        return Response::RespContextError;
    }
    if (!deck) {
        return Response::RespContextError;
    }
    if (sideboardLocked == cmd.locked()) {
        return Response::RespContextError;
    }

    sideboardLocked = cmd.locked();
    if (sideboardLocked) {
        deck->setCurrentSideboardPlan(QList<MoveCard_ToZone>());
    }

    Event_PlayerPropertiesChanged event;
    event.mutable_player_properties()->set_sideboard_locked(sideboardLocked);
    ges.enqueueGameEvent(event, playerId);
    ges.setGameEventContext(Context_SetSideboardLock());

    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdConcede(const Command_Concede & /*cmd*/, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator) {
        return Response::RespFunctionNotAllowed;
    }
    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }
    if (conceded) {
        return Response::RespContextError;
    }

    setConceded(true);
    game->removeArrowsRelatedToPlayer(ges, this);
    game->unattachCards(ges, this);
    clearZones();

    Event_PlayerPropertiesChanged event;
    event.mutable_player_properties()->set_conceded(true);
    ges.enqueueGameEvent(event, playerId);
    ges.setGameEventContext(Context_Concede());

    game->stopGameIfFinished();
    if (game->getGameStarted() && (game->getActivePlayer() == playerId)) {
        game->nextTurn();
    }

    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdUnconcede(const Command_Unconcede & /*cmd*/, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator) {
        return Response::RespFunctionNotAllowed;
    }
    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }
    if (!conceded) {
        return Response::RespContextError;
    }

    setConceded(false);

    Event_PlayerPropertiesChanged event;
    event.mutable_player_properties()->set_conceded(false);
    ges.enqueueGameEvent(event, playerId);
    ges.setGameEventContext(Context_Unconcede());

    setupZones();

    game->sendGameStateToPlayers();

    return Response::RespOk;
}

Response::ResponseCode Server_Player::cmdJudge(const Command_Judge &cmd, ResponseContainer &rc, GameEventStorage &ges)
{
    if (!judge)
        return Response::RespFunctionNotAllowed;

    Server_Player *player = this->game->getPlayers().value(cmd.target_id());

    ges.setForcedByJudge(playerId);
    if (player == nullptr)
        return Response::RespContextError;

    for (int i = 0; i < cmd.game_command_size(); ++i) {
        player->processGameCommand(cmd.game_command(i), rc, ges);
    }

    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdReadyStart(const Command_ReadyStart &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator) {
        return Response::RespFunctionNotAllowed;
    }

    if (!deck || game->getGameStarted()) {
        return Response::RespContextError;
    }

    if (readyStart == cmd.ready()) {
        return Response::RespContextError;
    }

    setReadyStart(cmd.ready());

    Event_PlayerPropertiesChanged event;
    event.mutable_player_properties()->set_ready_start(cmd.ready());
    ges.enqueueGameEvent(event, playerId);
    ges.setGameEventContext(Context_ReadyStart());

    if (cmd.ready()) {
        game->startGameIfReady();
    }

    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdGameSay(const Command_GameSay &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator) {
        /* Spectators can only talk if:
         * (a) the game creator allows it
         * (b) the spectator is a moderator/administrator
         * (c) the spectator is a judge
         */
        bool isModOrJudge = (userInfo->user_level() & (ServerInfo_User::IsModerator | ServerInfo_User::IsJudge));
        if (!isModOrJudge && !game->getSpectatorsCanTalk()) {
            return Response::RespFunctionNotAllowed;
        }
    }

    if (!userInterface->addSaidMessageSize(cmd.message().size())) {
        return Response::RespChatFlood;
    }
    Event_GameSay event;
    event.set_message(cmd.message());
    ges.enqueueGameEvent(event, playerId);

    game->getRoom()->getServer()->getDatabaseInterface()->logMessage(
        userInfo->id(), QString::fromStdString(userInfo->name()), QString::fromStdString(userInfo->address()),
        textFromStdString(cmd.message()), Server_DatabaseInterface::MessageTargetGame, game->getGameId(),
        game->getDescription());

    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdShuffle(const Command_Shuffle &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator) {
        return Response::RespFunctionNotAllowed;
    }

    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }

    if (conceded) {
        return Response::RespContextError;
    }

    auto zone = zones.value(nameFromStdString(cmd.has_zone_name() ? cmd.zone_name() : "deck"));
    if (!zone) {
        return Response::RespNameNotFound;
    }

    if (!zone->canShuffle()) {
        return Response::RespFunctionNotAllowed;
    }

    zone->shuffle(cmd.start(), cmd.end());

    Event_Shuffle event;
    event.set_zone_name(zone->getName().toStdString());
    event.set_start(cmd.start());
    event.set_end(cmd.end());
    ges.enqueueGameEvent(event, playerId);
    revealTopCardIfNeeded(zone, ges);

    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdMulligan(const Command_Mulligan &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator) {
        return Response::RespFunctionNotAllowed;
    }

    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }
    if (conceded) {
        return Response::RespContextError;
    }

    auto hand = zones.value("hand");
    auto deck = zones.value("deck");
    int number = cmd.number();

    if (!hand->getCards().isEmpty()) {
        lastDrawList.clear();
        moveCards(ges, hand->getCards(), deck, -1, 0);
    }

    deck->shuffle();
    ges.enqueueGameEvent(Event_Shuffle(), playerId);

    drawCards(ges, number);

    Context_Mulligan context;
    context.set_number(static_cast<google::protobuf::uint32>(hand->getCards().size()));
    ges.setGameEventContext(context);

    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdRollDie(const Command_RollDie &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator) {
        return Response::RespFunctionNotAllowed;
    }
    if (conceded) {
        return Response::RespContextError;
    }

    Event_RollDie event;
    event.set_sides(cmd.sides());
    event.set_value(rng->rand(1, cmd.sides()));
    ges.enqueueGameEvent(event, playerId);

    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdDrawCards(const Command_DrawCards &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator) {
        return Response::RespFunctionNotAllowed;
    }

    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }
    if (conceded) {
        return Response::RespContextError;
    }

    return drawCards(ges, cmd.number());
}

Response::ResponseCode
Server_Player::cmdUndoDraw(const Command_UndoDraw & /*cmd*/, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator) {
        return Response::RespFunctionNotAllowed;
    }

    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }
    if (conceded) {
        return Response::RespContextError;
    }

    if (lastDrawList.isEmpty()) {
        return Response::RespContextError;
    }

    auto handZone = zones.value("hand");
    auto deckZone = zones.value("deck");

    Server_Card *card = handZone->getCard(lastDrawList.takeLast());
    if (!card) {
        return Response::RespNameNotFound;
    }

    ges.setGameEventContext(Context_UndoDraw());
    moveCards(ges, {card}, deckZone, 0, 0);
    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdMoveCard(const Command_MoveCard &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator) {
        return Response::RespFunctionNotAllowed;
    }

    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }
    if (conceded) {
        return Response::RespContextError;
    }

    Server_Player *startPlayer = game->getPlayers().value(cmd.has_start_player_id() ? cmd.start_player_id() : playerId);
    if (!startPlayer) {
        return Response::RespNameNotFound;
    }
    auto startZone = startPlayer->getZones().value(nameFromStdString(cmd.start_zone()));
    if (!startZone) {
        return Response::RespNameNotFound;
    }

    if ((startPlayer != this) && (!startZone->getPlayersWithWritePermission().contains(playerId)) && !judge) {
        return Response::RespContextError;
    }

    Server_Player *targetPlayer = game->getPlayers().value(cmd.target_player_id());
    if (!targetPlayer) {
        return Response::RespNameNotFound;
    }
    auto targetZone = targetPlayer->getZones().value(nameFromStdString(cmd.target_zone()));
    if (!targetZone) {
        return Response::RespNameNotFound;
    }

    if ((startPlayer != this) && (targetPlayer != this) && !judge) {
        return Response::RespContextError;
    }

    // Disallow controller changes to other zones than the table.
    bool targetIsTable = targetZone->getType() == ZoneType::PublicZone && targetZone->hasCoords();
    bool controllerChanges = startZone->getPlayer() != targetZone->getPlayer();
    if (controllerChanges && !targetIsTable && !judge) {
        return Response::RespContextError;
    }

    QList<Server_Card *> cards;
    QSet<int> cardIdsToMove;
    QMap<int, CardAttributes> cardsAttributes;
    QMultiMap<int, std::pair<CardAttribute, QString>> cardsExtraAttributes;
    for (int i = 0; i < cmd.cards_to_move().card_size(); ++i) {
        const CardToMove *cardToMove = &cmd.cards_to_move().card(i);

        // The same card being moved twice would lead to undefined behaviour.
        if (cardIdsToMove.contains(cardToMove->card_id())) {
            continue;
        }
        cardIdsToMove.insert(cardToMove->card_id());

        // Consistency checks. In case the command contains illegal moves, try
        // to resolve the legal ones still.
        Server_Card *card = startZone->getCard(cardToMove->card_id());
        if (!card) {
            return Response::RespNameNotFound;
        }

        // Can't move attached cards
        if (card->getParentCard()) {
            continue;
        }

        // Can't move cards with attachments to a non-empty slot
        if (!card->getAttachedCards().isEmpty() && !targetZone->isColumnEmpty(cmd.x(), cmd.y())) {
            continue;
        }

        cards.append(card);

        // Face-down information is needed to determine visibility.
        if (cardToMove->has_face_down()) {
            cardsAttributes.insert(card->getId(), {cardToMove->face_down()});
        }

        if (cardToMove->has_pt()) {
            QString ptString = QString::fromStdString(cardToMove->pt());
            if (!ptString.isEmpty()) {
                cardsExtraAttributes.insert(card->getId(), {AttrPT, ptString});
            }
        }

        if (cardToMove->has_tapped()) {
            cardsExtraAttributes.insert(card->getId(), {AttrTapped, cardToMove->tapped() ? "1" : "0"});
        }
    }

    // In case all moves were filtered out, abort.
    if (cardIdsToMove.isEmpty()) {
        return Response::RespContextError;
    }

    // "Undo draw" should only remain valid if the just-drawn card stays within the user's hand (e.g., they only
    // reorder their hand). If a just-drawn card leaves the hand then remove cards before it from the list
    if (startZone->getName() == "hand" && targetZone != startZone) {
        for (const auto *card : cards) {
            int index = lastDrawList.lastIndexOf(card->getId());
            if (index != -1) {
                lastDrawList.erase(lastDrawList.begin(), lastDrawList.begin() + index);
            }
        }
    }

    ges.setGameEventContext(Context_MoveCard());
    moveCards(ges, cards, targetZone, cmd.x(), cmd.y(), cardsAttributes);

    for (const auto *card : cards) {
        for (auto [it, end] = cardsExtraAttributes.equal_range(card->getId()); it != end; ++it) {
            setCardAttrHelper(ges, card->getZone()->getPlayer()->getPlayerId(), card->getZone()->getName(),
                              card->getId(), it.value().first, it.value().second);
        }
    }

    startZone->fixFreeSpaces(ges);
    targetZone->fixFreeSpaces(ges);
    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdFlipCard(const Command_FlipCard &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator) {
        return Response::RespFunctionNotAllowed;
    }

    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }
    if (conceded) {
        return Response::RespContextError;
    }

    auto zone = zones.value(nameFromStdString(cmd.zone()));
    if (!zone) {
        return Response::RespNameNotFound;
    }
    if (!zone->hasCoords()) {
        return Response::RespContextError;
    }

    Server_Card *card = zone->getCard(cmd.card_id());
    if (!card) {
        return Response::RespNameNotFound;
    }

    const bool faceDown = cmd.face_down();
    if (faceDown == card->getFaceDown()) {
        return Response::RespContextError;
    }

    card->setFaceDown(faceDown);

    Event_FlipCard event;
    event.set_zone_name(zone->getName().toStdString());
    event.set_card_id(card->getId());
    if (!faceDown) {
        event.set_card_name(card->getName().toStdString());
    }
    event.set_face_down(faceDown);
    ges.enqueueGameEvent(event, playerId);

    QString ptString = nameFromStdString(cmd.pt());
    if (!ptString.isEmpty() && !faceDown) {
        setCardAttrHelper(ges, playerId, zone->getName(), card->getId(), AttrPT, ptString);
    }

    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdAttachCard(const Command_AttachCard &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator) {
        return Response::RespFunctionNotAllowed;
    }

    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }
    if (conceded) {
        return Response::RespContextError;
    }

    auto startzone = zones.value(nameFromStdString(cmd.start_zone()));
    if (!startzone) {
        return Response::RespNameNotFound;
    }

    Server_Card *card = startzone->getCard(cmd.card_id());
    if (!card) {
        return Response::RespNameNotFound;
    }

    Server_Player *targetPlayer = nullptr;
    std::shared_ptr<Server_CardZone> targetzone;
    Server_Card *targetCard = nullptr;

    if (cmd.has_target_player_id()) {
        targetPlayer = game->getPlayers().value(cmd.target_player_id());
        if (!targetPlayer) {
            return Response::RespNameNotFound;
        }
    } else if (!card->getParentCard()) {
        return Response::RespContextError;
    }
    if (targetPlayer) {
        targetzone = targetPlayer->getZones().value(nameFromStdString(cmd.target_zone()));
    }
    if (targetzone) {
        // This is currently enough to make sure cards don't get attached to a card that is not on the table.
        // Possibly a flag will have to be introduced for this sometime.
        if (!targetzone->hasCoords()) {
            return Response::RespContextError;
        }
        if (cmd.has_target_card_id()) {
            targetCard = targetzone->getCard(cmd.target_card_id());
        }
        if (targetCard) {
            if (targetCard->getParentCard()) {
                return Response::RespContextError;
            }
        } else {
            return Response::RespNameNotFound;
        }
    }
    if (!startzone->hasCoords()) {
        return Response::RespContextError;
    }

    QMapIterator<int, Server_Player *> playerIterator(game->getPlayers());
    while (playerIterator.hasNext()) {
        Server_Player *p = playerIterator.next().value();
        QList<Server_Arrow *> arrows = p->getArrows().values();
        QList<Server_Arrow *> toDelete;
        for (auto a : arrows) {
            auto *tCard = qobject_cast<Server_Card *>(a->getTargetItem());
            if ((tCard == card) || (a->getStartCard() == card)) {
                toDelete.append(a);
            }
        }
        for (auto &i : toDelete) {
            Event_DeleteArrow event;
            event.set_arrow_id(i->getId());
            ges.enqueueGameEvent(event, p->getPlayerId());
            p->deleteArrow(i->getId());
        }
    }

    if (targetCard) {
        // Unattach all cards attached to the card being attached.
        // Make a copy of the list because its contents change during the loop otherwise.
        QList<Server_Card *> attachedList = card->getAttachedCards();
        for (const auto &i : attachedList) {
            i->getZone()->getPlayer()->unattachCard(ges, i);
        }

        card->setParentCard(targetCard);
        const int oldX = card->getX();
        card->setCoords(-1, card->getY());
        startzone->updateCardCoordinates(card, oldX, card->getY());

        if (targetzone->isColumnStacked(targetCard->getX(), targetCard->getY())) {
            targetPlayer->moveCards(ges, {targetCard}, targetzone,
                                    targetzone->getFreeGridColumn(-2, targetCard->getY(), targetCard->getName(), false),
                                    targetCard->getY());

            if (targetCard->getFaceDown()) {
                targetzone->fixFreeSpaces(ges);
            }
        }

        Event_AttachCard event;
        event.set_start_zone(startzone->getName().toStdString());
        event.set_card_id(card->getId());
        event.set_target_player_id(targetPlayer->getPlayerId());
        event.set_target_zone(targetzone->getName().toStdString());
        event.set_target_card_id(targetCard->getId());
        ges.enqueueGameEvent(event, playerId);

        startzone->fixFreeSpaces(ges);
    } else {
        unattachCard(ges, card);
    }

    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdCreateToken(const Command_CreateToken &cmd, ResponseContainer &rc, GameEventStorage &ges)
{
    if (spectator) {
        return Response::RespFunctionNotAllowed;
    }

    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }
    if (conceded) {
        return Response::RespContextError;
    }

    auto zone = zones.value(nameFromStdString(cmd.zone()));
    if (!zone) {
        return Response::RespNameNotFound;
    }

    QString cardName = nameFromStdString(cmd.card_name());
    // If the token is a replacement for an existing card that must be moved
    // into a new zone, move the old card to the zone first to make space for
    // the token.
    std::shared_ptr<Server_CardZone> pileZone;
    Server_Card *parentCard = nullptr;
    QList<Server_Card *> cardsToAttach;
    if (cmd.has_target_pile()) {
        auto targetZone = cmd.has_target_zone() ? zones.value(nameFromStdString(cmd.target_zone())) : zone;
        if (!targetZone) {
            return Response::RespNameNotFound;
        }

        pileZone = createZone(cmd.target_pile());

        Event_CreateZone event;
        pileZone->copyConfig(event.mutable_config());
        ges.enqueueGameEvent(event, playerId);

        // If we have both a pile and a target card, put the card onto the pile.
        if (cmd.has_target_card_id()) {
            auto *targetCard = targetZone->getCard(cmd.target_card_id());
            if (!targetCard) {
                return Response::RespNameNotFound;
            }

            if (targetZone == zone) {
                parentCard = targetCard->getParentCard();
                QList<Server_Card *> attachedCards{targetCard->getAttachedCards()};
                for (auto *attachedCard : attachedCards) {
                    attachedCard->getZone()->getPlayer()->unattachCard(ges, attachedCard);
                }

                cardsToAttach.append(attachedCards);
            }

            moveCards(ges, {targetCard}, pileZone, -1, 0);
        }
    }

    int xCoord = cmd.has_x() ? cmd.x() : -1;
    int yCoord = cmd.has_y() ? cmd.y() : -1;

    if (yCoord < 0) {
        yCoord = 0;
    }

    if (zone->hasCoords()) {
        xCoord = zone->getFreeGridColumn(xCoord, yCoord, cardName, false);
    }

    if (xCoord < 0) {
        xCoord = 0;
    }

    auto *card = new Server_Card(cardName, newCardId(), xCoord, yCoord);
    card->moveToThread(thread());
    card->setPT(nameFromStdString(cmd.pt()));
    card->setColor(nameFromStdString(cmd.color()));
    card->setAnnotation(nameFromStdString(cmd.annotation()));
    card->setDestroyOnZoneChange(cmd.destroy_on_zone_change());

    zone->insertCard(card, xCoord, yCoord);

    Event_CreateToken event;
    event.set_zone_name(zone->getName().toStdString());
    event.set_card_id(card->getId());
    event.set_card_name(card->getName().toStdString());
    event.set_color(card->getColor().toStdString());
    event.set_pt(card->getPT().toStdString());
    event.set_annotation(card->getAnnotation().toStdString());
    event.set_destroy_on_zone_change(card->getDestroyOnZoneChange());
    event.set_x(xCoord);
    event.set_y(yCoord);
    ges.enqueueGameEvent(event, playerId);

    if (pileZone) {
        pileZone->attachToCard(card);

        Event_AttachZone event;
        pileZone->copyRef(event.mutable_ref());
        card->copyRef(event.mutable_parent());
        event.set_name(cmd.target_pile().name());
        ges.enqueueGameEvent(event, playerId);

        if (parentCard) {
            card->setParentCard(parentCard);

            Event_AttachCard event;
            event.set_start_zone(card->getZone()->getName().toStdString());
            event.set_card_id(card->getId());
            event.set_target_player_id(parentCard->getZone()->getPlayer()->getPlayerId());
            event.set_target_zone(parentCard->getZone()->getName().toStdString());
            event.set_target_card_id(parentCard->getId());
            ges.enqueueGameEvent(event, card->getZone()->getPlayer()->getPlayerId());
        }

        for (auto *cardToAttach : cardsToAttach) {
            cardToAttach->setParentCard(card);

            Event_AttachCard event;
            event.set_start_zone(cardToAttach->getZone()->getName().toStdString());
            event.set_card_id(cardToAttach->getId());
            event.set_target_player_id(card->getZone()->getPlayer()->getPlayerId());
            event.set_target_zone(card->getZone()->getName().toStdString());
            event.set_target_card_id(card->getId());
            ges.enqueueGameEvent(event, cardToAttach->getZone()->getPlayer()->getPlayerId());
        }

        return Response::RespOk;
    }

    // check if the token is a replacement for an existing card
    if (cmd.target_card_id() < 0) {
        return Response::RespOk;
    }

    Command_AttachCard cmd2;
    cmd2.set_start_zone(cmd.target_zone());
    cmd2.set_card_id(cmd.target_card_id());

    cmd2.set_target_player_id(zone->getPlayer()->getPlayerId());
    cmd2.set_target_zone(cmd.zone());
    cmd2.set_target_card_id(card->getId());

    return cmdAttachCard(cmd2, rc, ges);
}

Response::ResponseCode
Server_Player::cmdCreateArrow(const Command_CreateArrow &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator) {
        return Response::RespFunctionNotAllowed;
    }

    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }
    if (conceded) {
        return Response::RespContextError;
    }

    Server_Player *startPlayer = game->getPlayers().value(cmd.start_player_id());
    Server_Player *targetPlayer = game->getPlayers().value(cmd.target_player_id());
    if (!startPlayer || !targetPlayer) {
        return Response::RespNameNotFound;
    }
    QString startZoneName = nameFromStdString(cmd.start_zone());
    auto startZone = startPlayer->getZones().value(startZoneName);
    bool playerTarget = !cmd.has_target_zone();
    std::shared_ptr<Server_CardZone> targetZone;
    if (!playerTarget) {
        targetZone = targetPlayer->getZones().value(nameFromStdString(cmd.target_zone()));
    }
    if (!startZone || (!targetZone && !playerTarget)) {
        return Response::RespNameNotFound;
    }
    if (startZone->getType() != ZoneType::PublicZone) {
        return Response::RespContextError;
    }
    Server_Card *startCard = startZone->getCard(cmd.start_card_id());
    if (!startCard) {
        return Response::RespNameNotFound;
    }
    Server_Card *targetCard = nullptr;
    if (!playerTarget) {
        if (targetZone->getType() != ZoneType::PublicZone) {
            return Response::RespContextError;
        }
        targetCard = targetZone->getCard(cmd.target_card_id());
    }

    Server_ArrowTarget *targetItem;
    if (playerTarget) {
        targetItem = targetPlayer;
    } else {
        targetItem = targetCard;
    }
    if (!targetItem) {
        return Response::RespNameNotFound;
    }

    QMapIterator<int, Server_Arrow *> arrowIterator(arrows);
    while (arrowIterator.hasNext()) {
        Server_Arrow *temp = arrowIterator.next().value();
        if ((temp->getStartCard() == startCard) && (temp->getTargetItem() == targetItem)) {
            return Response::RespContextError;
        }
    }

    auto arrow = new Server_Arrow(newArrowId(), startCard, targetItem, cmd.arrow_color());
    addArrow(arrow);

    Event_CreateArrow event;
    ServerInfo_Arrow *arrowInfo = event.mutable_arrow_info();
    arrowInfo->set_id(arrow->getId());
    arrowInfo->set_start_player_id(startPlayer->getPlayerId());
    arrowInfo->set_start_zone(startZoneName.toStdString());
    arrowInfo->set_start_card_id(startCard->getId());
    arrowInfo->set_target_player_id(targetPlayer->getPlayerId());
    if (!playerTarget) {
        arrowInfo->set_target_zone(cmd.target_zone());
        arrowInfo->set_target_card_id(cmd.target_card_id());
    }
    arrowInfo->mutable_arrow_color()->CopyFrom(cmd.arrow_color());
    ges.enqueueGameEvent(event, playerId);

    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdDeleteArrow(const Command_DeleteArrow &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator) {
        return Response::RespFunctionNotAllowed;
    }

    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }
    if (conceded) {
        return Response::RespContextError;
    }

    if (!deleteArrow(cmd.arrow_id())) {
        return Response::RespNameNotFound;
    }

    Event_DeleteArrow event;
    event.set_arrow_id(cmd.arrow_id());
    ges.enqueueGameEvent(event, playerId);

    return Response::RespOk;
}

void Server_Player::destroyCard(GameEventStorage &ges, Server_Card *card)
{
    destroyCard(ges, card, card->getZone(), card->getX(), card->getY());
}

void Server_Player::destroyCard(GameEventStorage &ges,
                                Server_Card *card,
                                const std::shared_ptr<Server_CardZone> &targetZone,
                                int targetX,
                                int targetY)
{
    auto sourceZone = card->getZone();
    Server_Player *sourcePlayer = sourceZone->getPlayer();

    game->removeArrowsRelatedToCard(ges, card);
    game->unattachCardsRelatedToCard(ges, card);

    // Emit the DestroyCard event first, because we may be moving another card
    // to the space it occupies and that could confuse clients.
    Event_DestroyCard event;
    event.set_zone_name(sourceZone->getName().toStdString());
    event.set_card_id(card->getId());
    ges.enqueueGameEvent(event, sourcePlayer->getPlayerId());

    sourceZone->removeCard(card);

    // Note: a card could be moved to its own attached zone, either directly or
    // because it moves to an attached zone of a card in its attached zone etc.
    //
    // Let's try not to unintentionally lose cards in this situation...
    auto attachedZones = card->takeAttachedZones();
    for (const auto &attachedZone : attachedZones) {
        if (attachedZone == targetZone) {
            continue;
        }

        moveCards(ges, attachedZone->getCards(), targetZone, targetX, targetY);
    }

    for (const auto &attachedZone : attachedZones) {
        if (!attachedZone->getCards().isEmpty()) {
            moveCards(ges, attachedZone->getCards(), attachedZone->getPlayer()->zones.value("table"), -1, 0);
        }

        Event_DestroyZone event;
        attachedZone->copyRef(event.mutable_ref());
        ges.enqueueGameEvent(event, playerId);

        attachedZone->getPlayer()->destroyZone(attachedZone->getName());
    }

    card->deleteLater();
}

Response::ResponseCode
Server_Player::cmdSetCardAttr(const Command_SetCardAttr &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator) {
        return Response::RespFunctionNotAllowed;
    }

    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }
    if (conceded) {
        return Response::RespContextError;
    }

    return setCardAttrHelper(ges, playerId, nameFromStdString(cmd.zone()), cmd.card_id(), cmd.attribute(),
                             nameFromStdString(cmd.attr_value()));
}

Response::ResponseCode
Server_Player::cmdSetCardCounter(const Command_SetCardCounter &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator) {
        return Response::RespFunctionNotAllowed;
    }

    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }
    if (conceded) {
        return Response::RespContextError;
    }

    auto zone = zones.value(nameFromStdString(cmd.zone()));
    if (!zone) {
        return Response::RespNameNotFound;
    }
    if (!zone->hasCoords()) {
        return Response::RespContextError;
    }

    Server_Card *card = zone->getCard(cmd.card_id());
    if (!card) {
        return Response::RespNameNotFound;
    }

    card->setCounter(cmd.counter_id(), cmd.counter_value());

    Event_SetCardCounter event;
    event.set_zone_name(zone->getName().toStdString());
    event.set_card_id(card->getId());
    event.set_counter_id(cmd.counter_id());
    event.set_counter_value(cmd.counter_value());
    ges.enqueueGameEvent(event, playerId);

    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdIncCardCounter(const Command_IncCardCounter &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator) {
        return Response::RespFunctionNotAllowed;
    }

    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }
    if (conceded) {
        return Response::RespContextError;
    }

    auto zone = zones.value(nameFromStdString(cmd.zone()));
    if (!zone) {
        return Response::RespNameNotFound;
    }
    if (!zone->hasCoords()) {
        return Response::RespContextError;
    }

    Server_Card *card = zone->getCard(cmd.card_id());
    if (!card) {
        return Response::RespNameNotFound;
    }

    int newValue = card->getCounter(cmd.counter_id()) + cmd.counter_delta();
    card->setCounter(cmd.counter_id(), newValue);

    Event_SetCardCounter event;
    event.set_zone_name(zone->getName().toStdString());
    event.set_card_id(card->getId());
    event.set_counter_id(cmd.counter_id());
    event.set_counter_value(newValue);
    ges.enqueueGameEvent(event, playerId);

    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdIncCounter(const Command_IncCounter &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator) {
        return Response::RespFunctionNotAllowed;
    }

    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }
    if (conceded) {
        return Response::RespContextError;
    }

    Server_Counter *c = counters.value(cmd.counter_id(), 0);
    if (!c) {
        return Response::RespNameNotFound;
    }

    c->setCount(c->getCount() + cmd.delta());

    Event_SetCounter event;
    event.set_counter_id(c->getId());
    event.set_value(c->getCount());
    ges.enqueueGameEvent(event, playerId);

    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdCreateCounter(const Command_CreateCounter &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator) {
        return Response::RespFunctionNotAllowed;
    }

    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }
    if (conceded) {
        return Response::RespContextError;
    }

    auto *c = new Server_Counter(newCounterId(), nameFromStdString(cmd.counter_name()), cmd.counter_color(),
                                 cmd.radius(), cmd.value());
    addCounter(c);

    Event_CreateCounter event;
    ServerInfo_Counter *counterInfo = event.mutable_counter_info();
    counterInfo->set_id(c->getId());
    counterInfo->set_name(c->getName().toStdString());
    counterInfo->mutable_counter_color()->CopyFrom(cmd.counter_color());
    counterInfo->set_radius(c->getRadius());
    counterInfo->set_count(c->getCount());
    ges.enqueueGameEvent(event, playerId);

    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdSetCounter(const Command_SetCounter &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator) {
        return Response::RespFunctionNotAllowed;
    }

    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }
    if (conceded) {
        return Response::RespContextError;
    }

    Server_Counter *c = counters.value(cmd.counter_id(), 0);
    if (!c) {
        return Response::RespNameNotFound;
    }

    c->setCount(cmd.value());

    Event_SetCounter event;
    event.set_counter_id(c->getId());
    event.set_value(c->getCount());
    ges.enqueueGameEvent(event, playerId);

    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdDelCounter(const Command_DelCounter &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator) {
        return Response::RespFunctionNotAllowed;
    }

    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }
    if (conceded) {
        return Response::RespContextError;
    }

    Server_Counter *counter = counters.value(cmd.counter_id(), 0);
    if (!counter) {
        return Response::RespNameNotFound;
    }
    counters.remove(cmd.counter_id());
    delete counter;

    Event_DelCounter event;
    event.set_counter_id(cmd.counter_id());
    ges.enqueueGameEvent(event, playerId);

    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdNextTurn(const Command_NextTurn & /*cmd*/, ResponseContainer & /*rc*/, GameEventStorage & /*ges*/)
{
    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }

    if (!judge) {
        if (spectator) {
            return Response::RespFunctionNotAllowed;
        }

        if (conceded) {
            return Response::RespContextError;
        }
    }

    game->nextTurn();
    return Response::RespOk;
}

Response::ResponseCode Server_Player::cmdSetActivePhase(const Command_SetActivePhase &cmd,
                                                        ResponseContainer & /*rc*/,
                                                        GameEventStorage & /*ges*/)
{
    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }

    if (!judge) {
        if (spectator) {
            return Response::RespFunctionNotAllowed;
        }

        if (conceded) {
            return Response::RespContextError;
        }

        if (game->getActivePlayer() != playerId) {
            return Response::RespContextError;
        }
    }

    game->setActivePhase(cmd.phase());

    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdDumpZone(const Command_DumpZone &cmd, ResponseContainer &rc, GameEventStorage &ges)
{
    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }

    Server_Player *otherPlayer = game->getPlayers().value(cmd.player_id());
    if (!otherPlayer) {
        return Response::RespNameNotFound;
    }
    auto zone = otherPlayer->getZones().value(nameFromStdString(cmd.zone_name()));
    if (!zone) {
        return Response::RespNameNotFound;
    }
    if (!((zone->getType() == ZoneType::PublicZone) || (this == otherPlayer))) {
        return Response::RespContextError;
    }

    int numberCards = cmd.number_cards();
    const QList<Server_Card *> &cards = zone->getCards();

    auto *re = new Response_DumpZone;
    ServerInfo_Zone *zoneInfo = re->mutable_zone_info();
    zoneInfo->set_name(zone->getName().toStdString());
    zoneInfo->set_type(zone->getType());
    zoneInfo->set_with_coords(zone->hasCoords());
    zoneInfo->set_card_count(numberCards < cards.size() ? cards.size() : numberCards);

    for (int i = 0; (i < cards.size()) && (i < numberCards || numberCards == -1); ++i) {
        Server_Card *card = cards[i];
        QString displayedName = card->getFaceDown() ? QString() : card->getName();
        ServerInfo_Card *cardInfo = zoneInfo->add_card_list();
        cardInfo->set_name(displayedName.toStdString());
        if (zone->getType() == ZoneType::HiddenZone) {
            cardInfo->set_id(i);
        } else {
            cardInfo->set_id(card->getId());
            cardInfo->set_x(card->getX());
            cardInfo->set_y(card->getY());
            cardInfo->set_face_down(card->getFaceDown());
            cardInfo->set_tapped(card->getTapped());
            cardInfo->set_attacking(card->getAttacking());
            cardInfo->set_color(card->getColor().toStdString());
            cardInfo->set_pt(card->getPT().toStdString());
            cardInfo->set_annotation(card->getAnnotation().toStdString());
            cardInfo->set_destroy_on_zone_change(card->getDestroyOnZoneChange());
            cardInfo->set_doesnt_untap(card->getDoesntUntap());

            QMapIterator<int, int> cardCounterIterator(card->getCounters());
            while (cardCounterIterator.hasNext()) {
                cardCounterIterator.next();
                ServerInfo_CardCounter *counterInfo = cardInfo->add_counter_list();
                counterInfo->set_id(cardCounterIterator.key());
                counterInfo->set_value(cardCounterIterator.value());
            }

            if (card->getParentCard()) {
                cardInfo->set_attach_player_id(card->getParentCard()->getZone()->getPlayer()->getPlayerId());
                cardInfo->set_attach_zone(card->getParentCard()->getZone()->getName().toStdString());
                cardInfo->set_attach_card_id(card->getParentCard()->getId());
            }
        }
    }
    if (zone->getType() == ZoneType::HiddenZone) {
        zone->setCardsBeingLookedAt(numberCards);

        Event_DumpZone event;
        event.set_zone_owner_id(otherPlayer->getPlayerId());
        event.set_zone_name(zone->getName().toStdString());
        event.set_number_cards(numberCards);
        ges.enqueueGameEvent(event, playerId);
    }
    rc.setResponseExtension(re);
    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdRevealCards(const Command_RevealCards &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator) {
        return Response::RespFunctionNotAllowed;
    }

    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }
    if (conceded) {
        return Response::RespContextError;
    }

    if (cmd.has_player_id()) {
        Server_Player *otherPlayer = game->getPlayers().value(cmd.player_id());
        if (!otherPlayer)
            return Response::RespNameNotFound;
    }
    auto zone = zones.value(nameFromStdString(cmd.zone_name()));
    if (!zone) {
        return Response::RespNameNotFound;
    }

    QList<Server_Card *> cardsToReveal;
    if (cmd.top_cards() != -1) {
        for (int i = 0; i < cmd.top_cards(); i++) {
            Server_Card *card = zone->getCard(i);
            if (!card) {
                return Response::RespNameNotFound;
            }
            cardsToReveal.append(card);
        }
    } else if (cmd.card_id_size() == 0) {
        cardsToReveal = zone->getCards();
    } else if (cmd.card_id_size() == 1 && cmd.card_id(0) == -2) {
        // If there is a single card_id with value -2 (ie
        // Player::RANDOM_CARD_FROM_ZONE), pick a random card.
        //
        // This is to be compatible with clients supporting a single card_id
        // value, which send value -2 to request a random card.
        if (zone->getCards().isEmpty()) {
            return Response::RespContextError;
        }

        cardsToReveal.append(zone->getCards().at(rng->rand(0, zone->getCards().size() - 1)));
    } else {
        for (auto cardId : cmd.card_id()) {
            Server_Card *card = zone->getCard(cardId);
            if (!card) {
                return Response::RespNameNotFound;
            }
            cardsToReveal.append(card);
        }
    }

    Event_RevealCards eventOthers;
    eventOthers.set_grant_write_access(cmd.grant_write_access());
    eventOthers.set_zone_name(zone->getName().toStdString());
    eventOthers.set_number_of_cards(cardsToReveal.size());
    for (auto cardId : cmd.card_id()) {
        eventOthers.add_card_id(cardId);
    }
    if (cmd.has_player_id()) {
        eventOthers.set_other_player_id(cmd.player_id());
    }

    Event_RevealCards eventPrivate(eventOthers);

    for (auto card : cardsToReveal) {
        ServerInfo_Card *cardInfo = eventPrivate.add_cards();

        cardInfo->set_id(card->getId());
        cardInfo->set_name(card->getName().toStdString());
        cardInfo->set_x(card->getX());
        cardInfo->set_y(card->getY());
        cardInfo->set_face_down(card->getFaceDown());
        cardInfo->set_tapped(card->getTapped());
        cardInfo->set_attacking(card->getAttacking());
        cardInfo->set_color(card->getColor().toStdString());
        cardInfo->set_pt(card->getPT().toStdString());
        cardInfo->set_annotation(card->getAnnotation().toStdString());
        cardInfo->set_destroy_on_zone_change(card->getDestroyOnZoneChange());
        cardInfo->set_doesnt_untap(card->getDoesntUntap());

        QMapIterator<int, int> cardCounterIterator(card->getCounters());
        while (cardCounterIterator.hasNext()) {
            cardCounterIterator.next();
            ServerInfo_CardCounter *counterInfo = cardInfo->add_counter_list();
            counterInfo->set_id(cardCounterIterator.key());
            counterInfo->set_value(cardCounterIterator.value());
        }

        if (card->getParentCard()) {
            cardInfo->set_attach_player_id(card->getParentCard()->getZone()->getPlayer()->getPlayerId());
            cardInfo->set_attach_zone(card->getParentCard()->getZone()->getName().toStdString());
            cardInfo->set_attach_card_id(card->getParentCard()->getId());
        }
    }

    if (cmd.has_player_id()) {
        if (cmd.grant_write_access()) {
            zone->addWritePermission(cmd.player_id());
        }

        ges.enqueueGameEvent(eventPrivate, playerId, GameEventStorageItem::SendToPrivate, cmd.player_id());
        ges.enqueueGameEvent(eventOthers, playerId, GameEventStorageItem::SendToOthers);
    } else {
        if (cmd.grant_write_access()) {
            const QList<int> &playerIds = game->getPlayers().keys();
            for (int playerId : playerIds) {
                zone->addWritePermission(playerId);
            }
        }

        ges.enqueueGameEvent(eventPrivate, playerId);
    }

    return Response::RespOk;
}

Response::ResponseCode Server_Player::cmdChangeZoneProperties(const Command_ChangeZoneProperties &cmd,
                                                              ResponseContainer & /* rc */,
                                                              GameEventStorage &ges)
{
    auto zone = zones.value(nameFromStdString(cmd.zone_name()));
    if (!zone) {
        return Response::RespNameNotFound;
    }

    Event_ChangeZoneProperties event;
    event.set_zone_name(cmd.zone_name());

    // Neither value set -> error.
    if (!cmd.has_always_look_at_top_card() && !cmd.has_always_reveal_top_card()) {
        return Response::RespContextError;
    }

    // Neither value changed -> error.
    bool alwaysRevealChanged =
        cmd.has_always_reveal_top_card() && zone->getAlwaysRevealTopCard() != cmd.always_reveal_top_card();
    bool alwaysLookAtTopChanged =
        cmd.has_always_look_at_top_card() && zone->getAlwaysLookAtTopCard() != cmd.always_look_at_top_card();
    if (!alwaysRevealChanged && !alwaysLookAtTopChanged) {
        return Response::RespContextError;
    }

    if (cmd.has_always_reveal_top_card()) {
        zone->setAlwaysRevealTopCard(cmd.always_reveal_top_card());
        event.set_always_reveal_top_card(cmd.always_reveal_top_card());
    }
    if (cmd.has_always_look_at_top_card()) {
        zone->setAlwaysLookAtTopCard(cmd.always_look_at_top_card());
        event.set_always_look_at_top_card(cmd.always_look_at_top_card());
    }
    ges.enqueueGameEvent(event, playerId);
    revealTopCardIfNeeded(zone, ges);
    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdReverseTurn(const Command_ReverseTurn & /*cmd*/, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator) {
        return Response::RespFunctionNotAllowed;
    }

    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }

    if (conceded) {
        return Response::RespContextError;
    }

    bool reversedTurn = game->reverseTurnOrder();

    Event_ReverseTurn event;
    event.set_reversed(reversedTurn);
    ges.enqueueGameEvent(event, playerId);

    return Response::RespOk;
}

std::shared_ptr<Server_CardZone> Server_Player::createZone(const ZoneConfig &zoneConfig)
{
    // Make sure we use a name that doesn't already exist
    QString originalName = nameFromStdString(zoneConfig.has_name() ? zoneConfig.name() : "pile");
    QString name{originalName};
    int counter = zones.size();
    while (zones.value(name)) {
        name = originalName + QString::number(counter++);
    }

    Server_CardZone::ZoneFlags flags{Server_CardZone::DynamicZone};
    if (zoneConfig.flags() & ZoneConfig::SHUFFLE_FLAG) {
        flags |= Server_CardZone::CanShuffle;
    }

    auto zoneController = game->getPlayers().value(zoneConfig.player_id());
    if (!zoneController) {
        zoneController = this;
    }

    auto zone = Server_CardZone::create(this, name, zoneConfig.type(), flags);
    addZone(zone);

    return zone;
}

Response::ResponseCode
Server_Player::processGameCommand(const GameCommand &command, ResponseContainer &rc, GameEventStorage &ges)
{
    switch ((GameCommand::GameCommandType)getPbExtension(command)) {
        case GameCommand::KICK_FROM_GAME:
            return cmdKickFromGame(command.GetExtension(Command_KickFromGame::ext), rc, ges);
            break;
        case GameCommand::LEAVE_GAME:
            return cmdLeaveGame(command.GetExtension(Command_LeaveGame::ext), rc, ges);
            break;
        case GameCommand::GAME_SAY:
            return cmdGameSay(command.GetExtension(Command_GameSay::ext), rc, ges);
            break;
        case GameCommand::SHUFFLE:
            return cmdShuffle(command.GetExtension(Command_Shuffle::ext), rc, ges);
            break;
        case GameCommand::MULLIGAN:
            return cmdMulligan(command.GetExtension(Command_Mulligan::ext), rc, ges);
            break;
        case GameCommand::ROLL_DIE:
            return cmdRollDie(command.GetExtension(Command_RollDie::ext), rc, ges);
            break;
        case GameCommand::DRAW_CARDS:
            return cmdDrawCards(command.GetExtension(Command_DrawCards::ext), rc, ges);
            break;
        case GameCommand::UNDO_DRAW:
            return cmdUndoDraw(command.GetExtension(Command_UndoDraw::ext), rc, ges);
            break;
        case GameCommand::FLIP_CARD:
            return cmdFlipCard(command.GetExtension(Command_FlipCard::ext), rc, ges);
            break;
        case GameCommand::ATTACH_CARD:
            return cmdAttachCard(command.GetExtension(Command_AttachCard::ext), rc, ges);
            break;
        case GameCommand::CREATE_TOKEN:
            return cmdCreateToken(command.GetExtension(Command_CreateToken::ext), rc, ges);
            break;
        case GameCommand::CREATE_ARROW:
            return cmdCreateArrow(command.GetExtension(Command_CreateArrow::ext), rc, ges);
            break;
        case GameCommand::DELETE_ARROW:
            return cmdDeleteArrow(command.GetExtension(Command_DeleteArrow::ext), rc, ges);
            break;
        case GameCommand::SET_CARD_ATTR:
            return cmdSetCardAttr(command.GetExtension(Command_SetCardAttr::ext), rc, ges);
            break;
        case GameCommand::SET_CARD_COUNTER:
            return cmdSetCardCounter(command.GetExtension(Command_SetCardCounter::ext), rc, ges);
            break;
        case GameCommand::INC_CARD_COUNTER:
            return cmdIncCardCounter(command.GetExtension(Command_IncCardCounter::ext), rc, ges);
            break;
        case GameCommand::READY_START:
            return cmdReadyStart(command.GetExtension(Command_ReadyStart::ext), rc, ges);
            break;
        case GameCommand::CONCEDE:
            return cmdConcede(command.GetExtension(Command_Concede::ext), rc, ges);
            break;
        case GameCommand::INC_COUNTER:
            return cmdIncCounter(command.GetExtension(Command_IncCounter::ext), rc, ges);
            break;
        case GameCommand::CREATE_COUNTER:
            return cmdCreateCounter(command.GetExtension(Command_CreateCounter::ext), rc, ges);
            break;
        case GameCommand::SET_COUNTER:
            return cmdSetCounter(command.GetExtension(Command_SetCounter::ext), rc, ges);
            break;
        case GameCommand::DEL_COUNTER:
            return cmdDelCounter(command.GetExtension(Command_DelCounter::ext), rc, ges);
            break;
        case GameCommand::NEXT_TURN:
            return cmdNextTurn(command.GetExtension(Command_NextTurn::ext), rc, ges);
            break;
        case GameCommand::SET_ACTIVE_PHASE:
            return cmdSetActivePhase(command.GetExtension(Command_SetActivePhase::ext), rc, ges);
            break;
        case GameCommand::DUMP_ZONE:
            return cmdDumpZone(command.GetExtension(Command_DumpZone::ext), rc, ges);
            break;
        case GameCommand::REVEAL_CARDS:
            return cmdRevealCards(command.GetExtension(Command_RevealCards::ext), rc, ges);
            break;
        case GameCommand::MOVE_CARD:
            return cmdMoveCard(command.GetExtension(Command_MoveCard::ext), rc, ges);
            break;
        case GameCommand::SET_SIDEBOARD_PLAN:
            return cmdSetSideboardPlan(command.GetExtension(Command_SetSideboardPlan::ext), rc, ges);
            break;
        case GameCommand::DECK_SELECT:
            return cmdDeckSelect(command.GetExtension(Command_DeckSelect::ext), rc, ges);
            break;
        case GameCommand::SET_SIDEBOARD_LOCK:
            return cmdSetSideboardLock(command.GetExtension(Command_SetSideboardLock::ext), rc, ges);
            break;
        case GameCommand::CHANGE_ZONE_PROPERTIES:
            return cmdChangeZoneProperties(command.GetExtension(Command_ChangeZoneProperties::ext), rc, ges);
            break;
        case GameCommand::UNCONCEDE:
            return cmdUnconcede(command.GetExtension(Command_Unconcede::ext), rc, ges);
            break;
        case GameCommand::JUDGE:
            return cmdJudge(command.GetExtension(Command_Judge::ext), rc, ges);
            break;
        case GameCommand::REVERSE_TURN:
            return cmdReverseTurn(command.GetExtension(Command_ReverseTurn::ext), rc, ges);
            break;
        default:
            return Response::RespInvalidCommand;
    }
}

void Server_Player::sendGameEvent(const GameEventContainer &cont)
{
    QMutexLocker locker(&playerMutex);

    if (userInterface) {
        userInterface->sendProtocolItem(cont);
    }
}

void Server_Player::setUserInterface(Server_AbstractUserInterface *_userInterface)
{
    playerMutex.lock();
    userInterface = _userInterface;
    playerMutex.unlock();

    pingTime = _userInterface ? 0 : -1;

    Event_PlayerPropertiesChanged event;
    event.mutable_player_properties()->set_ping_seconds(pingTime);

    GameEventStorage ges;
    ges.setGameEventContext(Context_ConnectionStateChanged());
    ges.enqueueGameEvent(event, playerId);
    ges.sendToGame(game);
}

void Server_Player::disconnectClient()
{
    if (!(userInfo->user_level() & ServerInfo_User::IsRegistered) || spectator) {
        game->removePlayer(this, Event_Leave::USER_DISCONNECTED);
    } else {
        setUserInterface(nullptr);
    }
}

void Server_Player::getInfo(ServerInfo_Player *info,
                            Server_Player *playerWhosAsking,
                            bool omniscient,
                            bool withUserInfo)
{
    getProperties(*info->mutable_properties(), withUserInfo);
    if (playerWhosAsking == this) {
        if (deck) {
            info->set_deck_list(deck->writeToString_Native().toStdString());
        }
    }

    for (Server_Arrow *arrow : arrows) {
        arrow->getInfo(info->add_arrow_list());
    }

    for (Server_Counter *counter : counters) {
        counter->getInfo(info->add_counter_list());
    }

    for (auto zone : zones) {
        zone->getInfo(info->add_zone_list(), playerWhosAsking, omniscient);
    }
}
