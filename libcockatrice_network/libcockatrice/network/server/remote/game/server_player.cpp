#include "server_player.h"

#include "../server.h"
#include "../server_abstractuserinterface.h"
#include "../server_database_interface.h"
#include "../server_room.h"
#include "server_arrow.h"
#include "server_card.h"
#include "server_cardzone.h"
#include "server_counter.h"
#include "server_game.h"
#include "server_move_card_struct.h"

#include <QDebug>
#include <QRegularExpression>
#include <algorithm>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/deck_list/deck_list_card_node.h>
#include <libcockatrice/protocol/get_pb_extension.h>
#include <libcockatrice/protocol/pb/command_attach_card.pb.h>
#include <libcockatrice/protocol/pb/command_change_zone_properties.pb.h>
#include <libcockatrice/protocol/pb/command_concede.pb.h>
#include <libcockatrice/protocol/pb/command_create_arrow.pb.h>
#include <libcockatrice/protocol/pb/command_create_counter.pb.h>
#include <libcockatrice/protocol/pb/command_create_token.pb.h>
#include <libcockatrice/protocol/pb/command_deck_select.pb.h>
#include <libcockatrice/protocol/pb/command_del_counter.pb.h>
#include <libcockatrice/protocol/pb/command_delete_arrow.pb.h>
#include <libcockatrice/protocol/pb/command_draw_cards.pb.h>
#include <libcockatrice/protocol/pb/command_dump_zone.pb.h>
#include <libcockatrice/protocol/pb/command_flip_card.pb.h>
#include <libcockatrice/protocol/pb/command_game_say.pb.h>
#include <libcockatrice/protocol/pb/command_inc_card_counter.pb.h>
#include <libcockatrice/protocol/pb/command_inc_counter.pb.h>
#include <libcockatrice/protocol/pb/command_kick_from_game.pb.h>
#include <libcockatrice/protocol/pb/command_leave_game.pb.h>
#include <libcockatrice/protocol/pb/command_move_card.pb.h>
#include <libcockatrice/protocol/pb/command_mulligan.pb.h>
#include <libcockatrice/protocol/pb/command_next_turn.pb.h>
#include <libcockatrice/protocol/pb/command_ready_start.pb.h>
#include <libcockatrice/protocol/pb/command_reveal_cards.pb.h>
#include <libcockatrice/protocol/pb/command_reverse_turn.pb.h>
#include <libcockatrice/protocol/pb/command_roll_die.pb.h>
#include <libcockatrice/protocol/pb/command_set_active_phase.pb.h>
#include <libcockatrice/protocol/pb/command_set_card_attr.pb.h>
#include <libcockatrice/protocol/pb/command_set_card_counter.pb.h>
#include <libcockatrice/protocol/pb/command_set_counter.pb.h>
#include <libcockatrice/protocol/pb/command_set_sideboard_lock.pb.h>
#include <libcockatrice/protocol/pb/command_set_sideboard_plan.pb.h>
#include <libcockatrice/protocol/pb/command_shuffle.pb.h>
#include <libcockatrice/protocol/pb/command_undo_draw.pb.h>
#include <libcockatrice/protocol/pb/context_concede.pb.h>
#include <libcockatrice/protocol/pb/context_connection_state_changed.pb.h>
#include <libcockatrice/protocol/pb/context_deck_select.pb.h>
#include <libcockatrice/protocol/pb/context_move_card.pb.h>
#include <libcockatrice/protocol/pb/context_mulligan.pb.h>
#include <libcockatrice/protocol/pb/context_ready_start.pb.h>
#include <libcockatrice/protocol/pb/context_set_sideboard_lock.pb.h>
#include <libcockatrice/protocol/pb/context_undo_draw.pb.h>
#include <libcockatrice/protocol/pb/event_attach_card.pb.h>
#include <libcockatrice/protocol/pb/event_change_zone_properties.pb.h>
#include <libcockatrice/protocol/pb/event_create_arrow.pb.h>
#include <libcockatrice/protocol/pb/event_create_counter.pb.h>
#include <libcockatrice/protocol/pb/event_create_token.pb.h>
#include <libcockatrice/protocol/pb/event_del_counter.pb.h>
#include <libcockatrice/protocol/pb/event_delete_arrow.pb.h>
#include <libcockatrice/protocol/pb/event_destroy_card.pb.h>
#include <libcockatrice/protocol/pb/event_draw_cards.pb.h>
#include <libcockatrice/protocol/pb/event_dump_zone.pb.h>
#include <libcockatrice/protocol/pb/event_flip_card.pb.h>
#include <libcockatrice/protocol/pb/event_game_say.pb.h>
#include <libcockatrice/protocol/pb/event_move_card.pb.h>
#include <libcockatrice/protocol/pb/event_player_properties_changed.pb.h>
#include <libcockatrice/protocol/pb/event_reveal_cards.pb.h>
#include <libcockatrice/protocol/pb/event_reverse_turn.pb.h>
#include <libcockatrice/protocol/pb/event_roll_die.pb.h>
#include <libcockatrice/protocol/pb/event_set_card_attr.pb.h>
#include <libcockatrice/protocol/pb/event_set_card_counter.pb.h>
#include <libcockatrice/protocol/pb/event_set_counter.pb.h>
#include <libcockatrice/protocol/pb/event_shuffle.pb.h>
#include <libcockatrice/protocol/pb/response.pb.h>
#include <libcockatrice/protocol/pb/response_deck_download.pb.h>
#include <libcockatrice/protocol/pb/response_dump_zone.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_player.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_user.pb.h>
#include <libcockatrice/rng/rng_abstract.h>
#include <libcockatrice/utility/color.h>
#include <libcockatrice/utility/trice_limits.h>

Server_Player::Server_Player(Server_Game *_game,
                             int _playerId,
                             const ServerInfo_User &_userInfo,
                             bool _judge,
                             Server_AbstractUserInterface *_userInterface)
    : Server_AbstractPlayer(_game, _playerId, _userInfo, _judge, _userInterface)
{
}

Server_Player::~Server_Player() = default;

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

void Server_Player::setupZones()
{
    Server_AbstractPlayer::setupZones();

    // This may need to be customized according to the game rules.
    // ------------------------------------------------------------------

    // Create zones
    auto *deckZone = new Server_CardZone(this, "deck", false, ServerInfo_Zone::HiddenZone);
    addZone(deckZone);
    auto *sbZone = new Server_CardZone(this, "sb", false, ServerInfo_Zone::HiddenZone);
    addZone(sbZone);
    addZone(new Server_CardZone(this, "table", true, ServerInfo_Zone::PublicZone));
    addZone(new Server_CardZone(this, "hand", false, ServerInfo_Zone::PrivateZone));
    addZone(new Server_CardZone(this, "stack", false, ServerInfo_Zone::PublicZone));
    addZone(new Server_CardZone(this, "grave", false, ServerInfo_Zone::PublicZone));
    addZone(new Server_CardZone(this, "rfg", false, ServerInfo_Zone::PublicZone));

    addCounter(new Server_Counter(0, "life", makeColor(255, 255, 255), 25, game->getStartingLifeTotal()));
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
    for (int i = 0; i < listRoot->size(); ++i) {
        auto *currentZone = dynamic_cast<InnerDecklistNode *>(listRoot->at(i));
        Server_CardZone *z;
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
                z->insertCard(new Server_Card(currentCard->toCardRef(), nextCardId++, 0, 0, z), -1, 0);
            }
        }
    }

    const QList<MoveCard_ToZone> &sideboardPlan = deck->getCurrentSideboardPlan();
    for (const auto &m : sideboardPlan) {
        const QString startZone = nameFromStdString(m.start_zone());
        const QString targetZone = nameFromStdString(m.target_zone());

        Server_CardZone *start, *target;
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
    Server_AbstractPlayer::clearZones();
    for (Server_Counter *counter : counters) {
        delete counter;
    }
    counters.clear();

    lastDrawList.clear();
}

void Server_Player::addCounter(Server_Counter *counter)
{
    counters.insert(counter->getId(), counter);
}

Response::ResponseCode Server_Player::drawCards(GameEventStorage &ges, int number)
{
    Server_CardZone *deckZone = zones.value("deck");
    Server_CardZone *handZone = zones.value("hand");
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
        cardInfo->set_provider_id(card->getProviderId().toStdString());
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

void Server_Player::onCardBeingMoved(GameEventStorage &ges,
                                     const MoveCardStruct &cardStruct,
                                     Server_CardZone *startzone,
                                     Server_CardZone *targetzone,
                                     bool undoingDraw)
{
    Server_AbstractPlayer::onCardBeingMoved(ges, cardStruct, startzone, targetzone, undoingDraw);

    Server_Card *card = cardStruct.card;

    // "Undo draw" should only remain valid if the just-drawn card stays within the user's hand (e.g., they only
    // reorder their hand). If a just-drawn card leaves the hand then remove cards before it from the list
    // (Ignore the case where the card is currently being un-drawn.)
    if (startzone->getName() == "hand" && targetzone->getName() != "hand" && !undoingDraw) {
        int index = lastDrawList.lastIndexOf(card->getId());
        if (index != -1) {
            lastDrawList.erase(lastDrawList.begin(), lastDrawList.begin() + index);
        }
    }
}

Response::ResponseCode
Server_Player::cmdDeckSelect(const Command_DeckSelect &cmd, ResponseContainer &rc, GameEventStorage &ges)
{
    if (game->getGameStarted()) {
        return Response::RespContextError;
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
    if (game->getShareDecklistsOnLoad()) {
        context.set_deck_list(deck->writeToString_Native().toStdString());
    }
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
Server_Player::cmdShuffle(const Command_Shuffle &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }

    if (conceded) {
        return Response::RespContextError;
    }

    if (cmd.has_zone_name() && cmd.zone_name() != "deck") {
        return Response::RespFunctionNotAllowed;
    }

    Server_CardZone *zone = zones.value("deck");
    if (!zone) {
        return Response::RespNameNotFound;
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
    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }
    if (conceded) {
        return Response::RespContextError;
    }

    Server_CardZone *hand = zones.value("hand");
    Server_CardZone *_deck = zones.value("deck");
    int number = cmd.number();

    if (!hand->getCards().isEmpty()) {
        auto cardsToMove = QList<const CardToMove *>();
        for (auto &card : hand->getCards()) {
            auto *cardToMove = new CardToMove;
            cardToMove->set_card_id(card->getId());
            cardsToMove.append(cardToMove);
        }
        moveCard(ges, hand, cardsToMove, _deck, -1, 0, false);
        qDeleteAll(cardsToMove);
    }

    _deck->shuffle();
    ges.enqueueGameEvent(Event_Shuffle(), playerId);

    drawCards(ges, number);

    Context_Mulligan context;
    context.set_number(static_cast<google::protobuf::uint32>(hand->getCards().size()));
    ges.setGameEventContext(context);

    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdDrawCards(const Command_DrawCards &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
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
    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }
    if (conceded) {
        return Response::RespContextError;
    }

    if (lastDrawList.isEmpty()) {
        return Response::RespContextError;
    }

    Response::ResponseCode retVal;
    auto *cardToMove = new CardToMove;
    cardToMove->set_card_id(lastDrawList.takeLast());
    retVal = moveCard(ges, zones.value("hand"), QList<const CardToMove *>() << cardToMove, zones.value("deck"), 0, 0,
                      false, true);
    delete cardToMove;

    return retVal;
}

Response::ResponseCode
Server_Player::cmdIncCounter(const Command_IncCounter &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
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

    if (conceded && !judge) {
        return Response::RespContextError;
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

Response::ResponseCode Server_Player::cmdChangeZoneProperties(const Command_ChangeZoneProperties &cmd,
                                                              ResponseContainer &rc,
                                                              GameEventStorage &ges)
{
    auto ret = Server_AbstractPlayer::cmdChangeZoneProperties(cmd, rc, ges);

    Server_CardZone *zone = zones.value(nameFromStdString(cmd.zone_name()));
    if (!zone) {
        return Response::RespNameNotFound;
    }

    revealTopCardIfNeeded(zone, ges);
    return ret;
}

Response::ResponseCode
Server_Player::cmdReverseTurn(const Command_ReverseTurn &cmd, ResponseContainer &rc, GameEventStorage &ges)
{

    if (!judge && conceded) {
        return Response::RespContextError;
    }
    return Server_AbstractParticipant::cmdReverseTurn(cmd, rc, ges);
}

void Server_Player::getInfo(ServerInfo_Player *info,
                            Server_AbstractParticipant *recipient,
                            bool omniscient,
                            bool withUserInfo)
{
    Server_AbstractPlayer::getInfo(info, recipient, omniscient, withUserInfo);

    for (Server_Counter *counter : counters) {
        counter->getInfo(info->add_counter_list());
    }
}
