#include "server_player.h"
#include "color.h"
#include "decklist.h"
#include "get_pb_extension.h"
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
#include "pb/command_roll_die.pb.h"
#include "pb/command_set_active_phase.pb.h"
#include "pb/command_set_card_attr.pb.h"
#include "pb/command_set_card_counter.pb.h"
#include "pb/command_set_counter.pb.h"
#include "pb/command_set_sideboard_lock.pb.h"
#include "pb/command_set_sideboard_plan.pb.h"
#include "pb/command_shuffle.pb.h"
#include "pb/command_stop_dump_zone.pb.h"
#include "pb/command_undo_draw.pb.h"
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
#include "pb/event_player_properties_changed.pb.h"
#include "pb/event_reveal_cards.pb.h"
#include "pb/event_roll_die.pb.h"
#include "pb/event_set_card_attr.pb.h"
#include "pb/event_set_card_counter.pb.h"
#include "pb/event_set_counter.pb.h"
#include "pb/event_shuffle.pb.h"
#include "pb/event_stop_dump_zone.pb.h"
#include "pb/response.pb.h"
#include "pb/response_deck_download.pb.h"
#include "pb/response_dump_zone.pb.h"
#include "pb/serverinfo_player.pb.h"
#include "pb/serverinfo_user.pb.h"

#include "pb/context_concede.pb.h"
#include "pb/context_connection_state_changed.pb.h"
#include "pb/context_deck_select.pb.h"
#include "pb/context_move_card.pb.h"
#include "pb/context_mulligan.pb.h"
#include "pb/context_ready_start.pb.h"
#include "pb/context_set_sideboard_lock.pb.h"
#include "pb/context_undo_draw.pb.h"

#include <QDebug>

Server_Player::Server_Player(Server_Game *_game,
                             int _playerId,
                             const ServerInfo_User &_userInfo,
                             bool _spectator,
                             Server_AbstractUserInterface *_userInterface)
    : ServerInfo_User_Container(_userInfo), game(_game), userInterface(_userInterface), deck(nullptr), pingTime(0),
      playerId(_playerId), spectator(_spectator), initialCards(0), nextCardId(0), readyStart(false), conceded(false),
      sideboardLocked(true)
{
}

Server_Player::~Server_Player() = default;

void Server_Player::prepareDestroy()
{
    delete deck;

    playerMutex.lock();
    if (userInterface)
        userInterface->playerRemovedFromGame(game);
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
        if (c->getId() > id)
            id = c->getId();
    }
    return id + 1;
}

int Server_Player::newArrowId() const
{
    int id = 0;
    QMapIterator<int, Server_Arrow *> i(arrows);
    while (i.hasNext()) {
        Server_Arrow *a = i.next().value();
        if (a->getId() > id)
            id = a->getId();
    }
    return id + 1;
}

void Server_Player::setupZones()
{
    // This may need to be customized according to the game rules.
    // ------------------------------------------------------------------

    // Create zones
    Server_CardZone *deckZone = new Server_CardZone(this, "deck", false, ServerInfo_Zone::HiddenZone);
    addZone(deckZone);
    Server_CardZone *sbZone = new Server_CardZone(this, "sb", false, ServerInfo_Zone::HiddenZone);
    addZone(sbZone);
    addZone(new Server_CardZone(this, "table", true, ServerInfo_Zone::PublicZone));
    addZone(new Server_CardZone(this, "hand", false, ServerInfo_Zone::PrivateZone));
    addZone(new Server_CardZone(this, "stack", false, ServerInfo_Zone::PublicZone));
    addZone(new Server_CardZone(this, "grave", false, ServerInfo_Zone::PublicZone));
    addZone(new Server_CardZone(this, "rfg", false, ServerInfo_Zone::PublicZone));

    addCounter(new Server_Counter(0, "life", makeColor(255, 255, 255), 25, 20));
    addCounter(new Server_Counter(1, "w", makeColor(255, 255, 150), 20, 0));
    addCounter(new Server_Counter(2, "u", makeColor(150, 150, 255), 20, 0));
    addCounter(new Server_Counter(3, "b", makeColor(150, 150, 150), 20, 0));
    addCounter(new Server_Counter(4, "r", makeColor(250, 150, 150), 20, 0));
    addCounter(new Server_Counter(5, "g", makeColor(150, 255, 150), 20, 0));
    addCounter(new Server_Counter(6, "x", makeColor(255, 255, 255), 20, 0));
    addCounter(new Server_Counter(7, "storm", makeColor(255, 150, 30), 20, 0));

    initialCards = 7;

    // ------------------------------------------------------------------

    // Assign card ids and create deck from deck list
    InnerDecklistNode *listRoot = deck->getRoot();
    nextCardId = 0;
    for (int i = 0; i < listRoot->size(); ++i) {
        auto *currentZone = dynamic_cast<InnerDecklistNode *>(listRoot->at(i));
        Server_CardZone *z;
        if (currentZone->getName() == DECK_ZONE_MAIN)
            z = deckZone;
        else if (currentZone->getName() == DECK_ZONE_SIDE)
            z = sbZone;
        else
            continue;

        for (int j = 0; j < currentZone->size(); ++j) {
            auto *currentCard = dynamic_cast<DecklistCardNode *>(currentZone->at(j));
            if (!currentCard)
                continue;
            for (int k = 0; k < currentCard->getNumber(); ++k)
                z->insertCard(new Server_Card(currentCard->getName(), nextCardId++, 0, 0, z), -1, 0);
        }
    }

    const QList<MoveCard_ToZone> &sideboardPlan = deck->getCurrentSideboardPlan();
    for (const auto &m : sideboardPlan) {
        const QString startZone = QString::fromStdString(m.start_zone());
        const QString targetZone = QString::fromStdString(m.target_zone());

        Server_CardZone *start, *target;
        if (startZone == DECK_ZONE_MAIN)
            start = deckZone;
        else if (startZone == DECK_ZONE_SIDE)
            start = sbZone;
        else
            continue;
        if (targetZone == DECK_ZONE_MAIN)
            target = deckZone;
        else if (targetZone == DECK_ZONE_SIDE)
            target = sbZone;
        else
            continue;

        for (int j = 0; j < start->getCards().size(); ++j)
            if (start->getCards()[j]->getName() == QString::fromStdString(m.card_name())) {
                Server_Card *card = start->getCard(j, nullptr, true);
                target->insertCard(card, -1, 0);
                break;
            }
    }

    deckZone->shuffle();
}

void Server_Player::clearZones()
{
    QMapIterator<QString, Server_CardZone *> zoneIterator(zones);
    while (zoneIterator.hasNext())
        delete zoneIterator.next().value();
    zones.clear();

    QMapIterator<int, Server_Counter *> counterIterator(counters);
    while (counterIterator.hasNext())
        delete counterIterator.next().value();
    counters.clear();

    QMapIterator<int, Server_Arrow *> arrowIterator(arrows);
    while (arrowIterator.hasNext())
        delete arrowIterator.next().value();
    arrows.clear();

    lastDrawList.clear();
}

void Server_Player::getProperties(ServerInfo_PlayerProperties &result, bool withUserInfo)
{
    result.set_player_id(playerId);
    if (withUserInfo)
        copyUserInfo(*(result.mutable_user_info()), true);
    result.set_spectator(spectator);
    if (!spectator) {
        result.set_conceded(conceded);
        result.set_sideboard_locked(sideboardLocked);
        result.set_ready_start(readyStart);
    }
    if (deck)
        result.set_deck_hash(deck->getDeckHash().toStdString());
    result.set_ping_seconds(pingTime);
}

void Server_Player::addZone(Server_CardZone *zone)
{
    zones.insert(zone->getName(), zone);
}

void Server_Player::addArrow(Server_Arrow *arrow)
{
    arrows.insert(arrow->getId(), arrow);
}

bool Server_Player::deleteArrow(int arrowId)
{
    Server_Arrow *arrow = arrows.value(arrowId, 0);
    if (!arrow)
        return false;
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
    Server_CardZone *deckZone = zones.value("deck");
    Server_CardZone *handZone = zones.value("hand");
    if (deckZone->getCards().size() < number)
        number = deckZone->getCards().size();

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

    if (deckZone->getAlwaysRevealTopCard() && !deckZone->getCards().isEmpty()) {
        Event_RevealCards revealEvent;
        revealEvent.set_zone_name(deckZone->getName().toStdString());
        revealEvent.set_card_id(0);
        deckZone->getCards().first()->getInfo(revealEvent.add_cards());

        ges.enqueueGameEvent(revealEvent, playerId);
    }

    return Response::RespOk;
}

class Server_Player::MoveCardCompareFunctor
{
private:
    int x;

public:
    explicit MoveCardCompareFunctor(int _x) : x(_x)
    {
    }
    inline bool operator()(QPair<Server_Card *, int> a, QPair<Server_Card *, int> b)
    {
        if (a.second < x) {
            if (b.second >= x)
                return false;
            else
                return (a.second > b.second);
        } else {
            if (b.second < x)
                return true;
            else
                return (a.second < b.second);
        }
    }
};

Response::ResponseCode Server_Player::moveCard(GameEventStorage &ges,
                                               Server_CardZone *startzone,
                                               const QList<const CardToMove *> &_cards,
                                               Server_CardZone *targetzone,
                                               int x,
                                               int y,
                                               bool fixFreeSpaces,
                                               bool undoingDraw)
{
    // Disallow controller change to other zones than the table.
    if (((targetzone->getType() != ServerInfo_Zone::PublicZone) || !targetzone->hasCoords()) &&
        (startzone->getPlayer() != targetzone->getPlayer()))
        return Response::RespContextError;

    if (!targetzone->hasCoords() && (x <= -1))
        x = targetzone->getCards().size();

    QList<QPair<Server_Card *, int>> cardsToMove;
    QMap<Server_Card *, const CardToMove *> cardProperties;
    QSet<int> cardIdsToMove;
    for (auto _card : _cards) {
        // The same card being moved twice would lead to undefined behaviour.
        if (cardIdsToMove.contains(_card->card_id()))
            continue;
        cardIdsToMove.insert(_card->card_id());

        // Consistency checks. In case the command contains illegal moves, try to resolve the legal ones still.
        int position;
        Server_Card *card = startzone->getCard(_card->card_id(), &position);
        if (!card)
            return Response::RespNameNotFound;
        if (card->getParentCard())
            continue;
        if (!card->getAttachedCards().isEmpty() && !targetzone->isColumnEmpty(x, y))
            continue;
        cardsToMove.append(QPair<Server_Card *, int>(card, position));
        cardProperties.insert(card, _card);
    }
    // In case all moves were filtered out, abort.
    if (cardsToMove.isEmpty())
        return Response::RespContextError;

    // 0 performs no sorting
    // 1 reverses the sorting
    MoveCardCompareFunctor cmp(0);
    qSort(cardsToMove.begin(), cardsToMove.end(), cmp);

    bool secondHalf = false;
    int xIndex = -1;

    for (int cardIndex = cardsToMove.size() - 1; cardIndex > -1; --cardIndex) {
        Server_Card *card = cardsToMove[cardIndex].first;
        const CardToMove *thisCardProperties = cardProperties.value(card);
        bool faceDown = thisCardProperties->has_face_down() ? thisCardProperties->face_down() : card->getFaceDown();
        if (!targetzone->hasCoords())
            faceDown = false;

        int originalPosition = cardsToMove[cardIndex].second;
        int position = startzone->removeCard(card);
        if (startzone->getName() == "hand") {
            if (undoingDraw)
                lastDrawList.removeAt(lastDrawList.indexOf(card->getId()));
            else if (lastDrawList.contains(card->getId()))
                lastDrawList.clear();
        }

        if ((startzone == targetzone) && !startzone->hasCoords()) {
            if (!secondHalf && (originalPosition < x)) {
                xIndex = -1;
                secondHalf = true;
            } else if (secondHalf)
                --xIndex;
            else
                ++xIndex;
        } else
            ++xIndex;
        int newX = x + xIndex;

        // Attachment relationships can be retained when moving a card onto the opponent's table
        if (startzone->getName() != targetzone->getName()) {
            // Delete all attachment relationships
            if (card->getParentCard())
                card->setParentCard(nullptr);

            // Make a copy of the list because the original one gets modified during the loop
            QList<Server_Card *> attachedCards = card->getAttachedCards();
            for (auto &attachedCard : attachedCards)
                attachedCard->getZone()->getPlayer()->unattachCard(ges, attachedCard);
        }

        if (startzone != targetzone) {
            // Delete all arrows from and to the card
            const QList<Server_Player *> &players = game->getPlayers().values();
            for (auto player : players) {
                QList<int> arrowsToDelete;
                QMapIterator<int, Server_Arrow *> arrowIterator(player->getArrows());
                while (arrowIterator.hasNext()) {
                    Server_Arrow *arrow = arrowIterator.next().value();
                    if ((arrow->getStartCard() == card) || (arrow->getTargetItem() == card))
                        arrowsToDelete.append(arrow->getId());
                }
                for (int j : arrowsToDelete)
                    player->deleteArrow(j);
            }
        }

        if (card->getDestroyOnZoneChange() && (startzone->getName() != targetzone->getName())) {
            Event_DestroyCard event;
            event.set_zone_name(startzone->getName().toStdString());
            event.set_card_id(static_cast<google::protobuf::uint32>(card->getId()));
            ges.enqueueGameEvent(event, playerId);

            card->deleteLater();
        } else {
            if (!targetzone->hasCoords()) {
                y = 0;
                card->resetState();
            } else
                newX = targetzone->getFreeGridColumn(newX, y, card->getName(), faceDown);

            targetzone->insertCard(card, newX, y);

            bool targetBeingLookedAt = (targetzone->getType() != ServerInfo_Zone::HiddenZone) ||
                                       (targetzone->getCardsBeingLookedAt() > newX) ||
                                       (targetzone->getCardsBeingLookedAt() == -1);
            bool sourceBeingLookedAt = (startzone->getType() != ServerInfo_Zone::HiddenZone) ||
                                       (startzone->getCardsBeingLookedAt() > position) ||
                                       (startzone->getCardsBeingLookedAt() == -1);

            bool targetHiddenToPlayer = faceDown || !targetBeingLookedAt;
            bool targetHiddenToOthers = faceDown || (targetzone->getType() != ServerInfo_Zone::PublicZone);
            bool sourceHiddenToPlayer = card->getFaceDown() || !sourceBeingLookedAt;
            bool sourceHiddenToOthers = card->getFaceDown() || (startzone->getType() != ServerInfo_Zone::PublicZone);

            QString privateCardName, publicCardName;
            if (!(sourceHiddenToPlayer && targetHiddenToPlayer))
                privateCardName = card->getName();
            if (!(sourceHiddenToOthers && targetHiddenToOthers))
                publicCardName = card->getName();

            int oldCardId = card->getId();
            if ((faceDown && (startzone != targetzone)) || (targetzone->getPlayer() != startzone->getPlayer()))
                card->setId(targetzone->getPlayer()->newCardId());
            card->setFaceDown(faceDown);

            // The player does not get to see which card he moved if it moves between two parts of hidden zones which
            // are not being looked at.
            int privateNewCardId = card->getId();
            int privateOldCardId = oldCardId;
            if (!targetBeingLookedAt && !sourceBeingLookedAt) {
                privateOldCardId = -1;
                privateNewCardId = -1;
                privateCardName = QString();
            }
            int privatePosition = -1;
            if (startzone->getType() == ServerInfo_Zone::HiddenZone)
                privatePosition = position;

            int publicNewX = newX;

            Event_MoveCard eventOthers;
            eventOthers.set_start_player_id(startzone->getPlayer()->getPlayerId());
            eventOthers.set_start_zone(startzone->getName().toStdString());
            eventOthers.set_target_player_id(targetzone->getPlayer()->getPlayerId());
            if (startzone != targetzone)
                eventOthers.set_target_zone(targetzone->getName().toStdString());
            eventOthers.set_y(y);
            eventOthers.set_face_down(faceDown);

            Event_MoveCard eventPrivate(eventOthers);
            eventPrivate.set_card_id(privateOldCardId);
            if (!privateCardName.isEmpty())
                eventPrivate.set_card_name(privateCardName.toStdString());
            eventPrivate.set_position(privatePosition);
            eventPrivate.set_new_card_id(privateNewCardId);
            eventPrivate.set_x(newX);

            // Other players do not get to see the start and/or target position of the card if the respective
            // part of the zone is being looked at. The information is not needed anyway because in hidden zones,
            // all cards are equal.
            if (((startzone->getType() == ServerInfo_Zone::HiddenZone) &&
                 ((startzone->getCardsBeingLookedAt() > position) || (startzone->getCardsBeingLookedAt() == -1))) ||
                (startzone->getType() == ServerInfo_Zone::PublicZone))
                position = -1;
            if ((targetzone->getType() == ServerInfo_Zone::HiddenZone) &&
                ((targetzone->getCardsBeingLookedAt() > newX) || (targetzone->getCardsBeingLookedAt() == -1)))
                publicNewX = -1;

            eventOthers.set_x(publicNewX);
            eventOthers.set_position(position);
            if ((startzone->getType() == ServerInfo_Zone::PublicZone) ||
                (targetzone->getType() == ServerInfo_Zone::PublicZone)) {
                eventOthers.set_card_id(oldCardId);
                if (!publicCardName.isEmpty())
                    eventOthers.set_card_name(publicCardName.toStdString());
                eventOthers.set_new_card_id(card->getId());
            }

            ges.enqueueGameEvent(eventPrivate, playerId, GameEventStorageItem::SendToPrivate, playerId);
            ges.enqueueGameEvent(eventOthers, playerId, GameEventStorageItem::SendToOthers);

            if (thisCardProperties->tapped())
                setCardAttrHelper(ges, targetzone->getPlayer()->getPlayerId(), targetzone->getName(), card->getId(),
                                  AttrTapped, "1");
            QString ptString = QString::fromStdString(thisCardProperties->pt());
            if (!ptString.isEmpty() && !faceDown)
                setCardAttrHelper(ges, targetzone->getPlayer()->getPlayerId(), targetzone->getName(), card->getId(),
                                  AttrPT, ptString);
        }
        if (startzone->getAlwaysRevealTopCard() && !startzone->getCards().isEmpty() && (originalPosition == 0)) {
            Event_RevealCards revealEvent;
            revealEvent.set_zone_name(startzone->getName().toStdString());
            revealEvent.set_card_id(0);
            startzone->getCards().first()->getInfo(revealEvent.add_cards());

            ges.enqueueGameEvent(revealEvent, playerId);
        }
        if (targetzone->getAlwaysRevealTopCard() && !targetzone->getCards().isEmpty() && (newX == 0)) {
            Event_RevealCards revealEvent;
            revealEvent.set_zone_name(targetzone->getName().toStdString());
            revealEvent.set_card_id(0);
            targetzone->getCards().first()->getInfo(revealEvent.add_cards());

            ges.enqueueGameEvent(revealEvent, playerId);
        }
    }
    if (undoingDraw)
        ges.setGameEventContext(Context_UndoDraw());
    else
        ges.setGameEventContext(Context_MoveCard());

    if (startzone->hasCoords() && fixFreeSpaces)
        startzone->fixFreeSpaces(ges);

    return Response::RespOk;
}

void Server_Player::unattachCard(GameEventStorage &ges, Server_Card *card)
{
    Server_CardZone *zone = card->getZone();
    Server_Card *parentCard = card->getParentCard();
    card->setParentCard(nullptr);

    Event_AttachCard event;
    event.set_start_zone(zone->getName().toStdString());
    event.set_card_id(card->getId());
    ges.enqueueGameEvent(event, playerId);

    auto *cardToMove = new CardToMove;
    cardToMove->set_card_id(card->getId());
    moveCard(ges, zone, QList<const CardToMove *>() << cardToMove, zone, -1, card->getY(), card->getFaceDown());
    delete cardToMove;

    if (parentCard->getZone())
        parentCard->getZone()->updateCardCoordinates(parentCard, parentCard->getX(), parentCard->getY());
}

Response::ResponseCode Server_Player::setCardAttrHelper(GameEventStorage &ges,
                                                        int targetPlayerId,
                                                        const QString &zoneName,
                                                        int cardId,
                                                        CardAttribute attribute,
                                                        const QString &attrValue)
{
    Server_CardZone *zone = getZones().value(zoneName);
    if (!zone)
        return Response::RespNameNotFound;
    if (!zone->hasCoords())
        return Response::RespContextError;

    QString result;
    if (cardId == -1) {
        QListIterator<Server_Card *> CardIterator(zone->getCards());
        while (CardIterator.hasNext()) {
            result = CardIterator.next()->setAttribute(attribute, attrValue, true);
            if (result.isNull())
                return Response::RespInvalidCommand;
        }
    } else {
        Server_Card *card = zone->getCard(cardId);
        if (!card)
            return Response::RespNameNotFound;
        result = card->setAttribute(attribute, attrValue, false);
        if (result.isNull())
            return Response::RespInvalidCommand;
    }

    Event_SetCardAttr event;
    event.set_zone_name(zone->getName().toStdString());
    if (cardId != -1)
        event.set_card_id(cardId);
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
    if ((game->getHostId() != playerId) && !(userInfo->user_level() & ServerInfo_User::IsModerator))
        return Response::RespFunctionNotAllowed;

    if (!game->kickPlayer(cmd.player_id()))
        return Response::RespNameNotFound;

    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdDeckSelect(const Command_DeckSelect &cmd, ResponseContainer &rc, GameEventStorage &ges)
{
    if (spectator)
        return Response::RespFunctionNotAllowed;

    DeckList *newDeck;
    if (cmd.has_deck_id()) {
        try {
            newDeck = game->getRoom()->getServer()->getDatabaseInterface()->getDeckFromDatabase(cmd.deck_id(),
                                                                                                userInfo->id());
        } catch (Response::ResponseCode &r) {
            return r;
        }
    } else
        newDeck = new DeckList(QString::fromStdString(cmd.deck()));

    if (!newDeck)
        return Response::RespInternalError;

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
    if (spectator)
        return Response::RespFunctionNotAllowed;
    if (readyStart)
        return Response::RespContextError;
    if (!deck)
        return Response::RespContextError;
    if (sideboardLocked)
        return Response::RespContextError;

    QList<MoveCard_ToZone> sideboardPlan;
    for (int i = 0; i < cmd.move_list_size(); ++i)
        sideboardPlan.append(cmd.move_list(i));
    deck->setCurrentSideboardPlan(sideboardPlan);

    return Response::RespOk;
}

Response::ResponseCode Server_Player::cmdSetSideboardLock(const Command_SetSideboardLock &cmd,
                                                          ResponseContainer & /*rc*/,
                                                          GameEventStorage &ges)
{
    if (spectator)
        return Response::RespFunctionNotAllowed;
    if (readyStart)
        return Response::RespContextError;
    if (!deck)
        return Response::RespContextError;
    if (sideboardLocked == cmd.locked())
        return Response::RespContextError;

    sideboardLocked = cmd.locked();
    if (sideboardLocked)
        deck->setCurrentSideboardPlan(QList<MoveCard_ToZone>());

    Event_PlayerPropertiesChanged event;
    event.mutable_player_properties()->set_sideboard_locked(sideboardLocked);
    ges.enqueueGameEvent(event, playerId);
    ges.setGameEventContext(Context_SetSideboardLock());

    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdConcede(const Command_Concede & /*cmd*/, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator)
        return Response::RespFunctionNotAllowed;
    if (!game->getGameStarted())
        return Response::RespGameNotStarted;
    if (conceded)
        return Response::RespContextError;

    setConceded(true);
    game->removeArrowsRelatedToPlayer(ges, this);
    game->unattachCards(ges, this);
    clearZones();

    Event_PlayerPropertiesChanged event;
    event.mutable_player_properties()->set_conceded(true);
    ges.enqueueGameEvent(event, playerId);
    ges.setGameEventContext(Context_Concede());

    game->stopGameIfFinished();
    if (game->getGameStarted() && (game->getActivePlayer() == playerId))
        game->nextTurn();

    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdUnconcede(const Command_Unconcede & /*cmd*/, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator)
        return Response::RespFunctionNotAllowed;
    if (!game->getGameStarted())
        return Response::RespGameNotStarted;
    if (!conceded)
        return Response::RespContextError;

    setConceded(false);

    Event_PlayerPropertiesChanged event;
    event.mutable_player_properties()->set_conceded(false);
    ges.enqueueGameEvent(event, playerId);
    ges.setGameEventContext(Context_Unconcede());

    setupZones();

    game->sendGameStateToPlayers();

    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdReadyStart(const Command_ReadyStart &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator)
        return Response::RespFunctionNotAllowed;

    if (!deck || game->getGameStarted())
        return Response::RespContextError;

    if (readyStart == cmd.ready())
        return Response::RespContextError;

    setReadyStart(cmd.ready());

    Event_PlayerPropertiesChanged event;
    event.mutable_player_properties()->set_ready_start(cmd.ready());
    ges.enqueueGameEvent(event, playerId);
    ges.setGameEventContext(Context_ReadyStart());

    if (cmd.ready())
        game->startGameIfReady();

    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdGameSay(const Command_GameSay &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator && !game->getSpectatorsCanTalk() && !(userInfo->user_level() & ServerInfo_User::IsModerator))
        return Response::RespFunctionNotAllowed;

    Event_GameSay event;
    event.set_message(cmd.message());
    ges.enqueueGameEvent(event, playerId);

    game->getRoom()->getServer()->getDatabaseInterface()->logMessage(
        userInfo->id(), QString::fromStdString(userInfo->name()), QString::fromStdString(userInfo->address()),
        QString::fromStdString(cmd.message()), Server_DatabaseInterface::MessageTargetGame, game->getGameId(),
        game->getDescription());

    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdShuffle(const Command_Shuffle &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator)
        return Response::RespFunctionNotAllowed;

    if (!game->getGameStarted())
        return Response::RespGameNotStarted;

    if (conceded)
        return Response::RespContextError;

    if (cmd.has_zone_name() && cmd.zone_name() != "deck")
        return Response::RespFunctionNotAllowed;

    Server_CardZone *zone = zones.value("deck");
    if (!zone)
        return Response::RespNameNotFound;

    zone->shuffle(cmd.start(), cmd.end());

    Event_Shuffle event;
    event.set_zone_name(zone->getName().toStdString());
    event.set_start(cmd.start());
    event.set_end(cmd.end());
    ges.enqueueGameEvent(event, playerId);

    if (zone->getAlwaysRevealTopCard() && !zone->getCards().isEmpty()) {
        Event_RevealCards revealEvent;
        revealEvent.set_zone_name(zone->getName().toStdString());
        revealEvent.set_card_id(0);
        zone->getCards().first()->getInfo(revealEvent.add_cards());

        ges.enqueueGameEvent(revealEvent, playerId);
    }

    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdMulligan(const Command_Mulligan & /*cmd*/, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator)
        return Response::RespFunctionNotAllowed;

    if (!game->getGameStarted())
        return Response::RespGameNotStarted;
    if (conceded)
        return Response::RespContextError;

    Server_CardZone *hand = zones.value("hand");
    int number = (hand->getCards().size() <= 1) ? initialCards : hand->getCards().size() - 1;

    Server_CardZone *deck = zones.value("deck");
    while (!hand->getCards().isEmpty()) {
        auto *cardToMove = new CardToMove;
        cardToMove->set_card_id(hand->getCards().first()->getId());
        moveCard(ges, hand, QList<const CardToMove *>() << cardToMove, deck, 0, 0, false);
        delete cardToMove;
    }

    deck->shuffle();
    ges.enqueueGameEvent(Event_Shuffle(), playerId);

    drawCards(ges, number);

    if (number == initialCards)
        number = -1;

    Context_Mulligan context;
    context.set_number(static_cast<google::protobuf::uint32>(number));
    ges.setGameEventContext(context);

    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdRollDie(const Command_RollDie &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator)
        return Response::RespFunctionNotAllowed;
    if (conceded)
        return Response::RespContextError;

    Event_RollDie event;
    event.set_sides(cmd.sides());
    event.set_value(rng->rand(1, cmd.sides()));
    ges.enqueueGameEvent(event, playerId);

    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdDrawCards(const Command_DrawCards &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator)
        return Response::RespFunctionNotAllowed;

    if (!game->getGameStarted())
        return Response::RespGameNotStarted;
    if (conceded)
        return Response::RespContextError;

    return drawCards(ges, cmd.number());
}

Response::ResponseCode
Server_Player::cmdUndoDraw(const Command_UndoDraw & /*cmd*/, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator)
        return Response::RespFunctionNotAllowed;

    if (!game->getGameStarted())
        return Response::RespGameNotStarted;
    if (conceded)
        return Response::RespContextError;

    if (lastDrawList.isEmpty())
        return Response::RespContextError;

    Response::ResponseCode retVal;
    auto *cardToMove = new CardToMove;
    cardToMove->set_card_id(lastDrawList.takeLast());
    retVal = moveCard(ges, zones.value("hand"), QList<const CardToMove *>() << cardToMove, zones.value("deck"), 0, 0,
                      false, true);
    delete cardToMove;

    return retVal;
}

Response::ResponseCode
Server_Player::cmdMoveCard(const Command_MoveCard &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator)
        return Response::RespFunctionNotAllowed;

    if (!game->getGameStarted())
        return Response::RespGameNotStarted;
    if (conceded)
        return Response::RespContextError;

    Server_Player *startPlayer = game->getPlayers().value(cmd.has_start_player_id() ? cmd.start_player_id() : playerId);
    if (!startPlayer)
        return Response::RespNameNotFound;
    Server_CardZone *startZone = startPlayer->getZones().value(QString::fromStdString(cmd.start_zone()));
    if (!startZone)
        return Response::RespNameNotFound;

    if ((startPlayer != this) && (!startZone->getPlayersWithWritePermission().contains(playerId)))
        return Response::RespContextError;

    Server_Player *targetPlayer = game->getPlayers().value(cmd.target_player_id());
    if (!targetPlayer)
        return Response::RespNameNotFound;
    Server_CardZone *targetZone = targetPlayer->getZones().value(QString::fromStdString(cmd.target_zone()));
    if (!targetZone)
        return Response::RespNameNotFound;

    if ((startPlayer != this) && (targetPlayer != this))
        return Response::RespContextError;

    QList<const CardToMove *> cardsToMove;
    for (int i = 0; i < cmd.cards_to_move().card_size(); ++i)
        cardsToMove.append(&cmd.cards_to_move().card(i));

    return moveCard(ges, startZone, cardsToMove, targetZone, cmd.x(), cmd.y());
}

Response::ResponseCode
Server_Player::cmdFlipCard(const Command_FlipCard &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator)
        return Response::RespFunctionNotAllowed;

    if (!game->getGameStarted())
        return Response::RespGameNotStarted;
    if (conceded)
        return Response::RespContextError;

    Server_CardZone *zone = zones.value(QString::fromStdString(cmd.zone()));
    if (!zone)
        return Response::RespNameNotFound;
    if (!zone->hasCoords())
        return Response::RespContextError;

    Server_Card *card = zone->getCard(cmd.card_id());
    if (!card)
        return Response::RespNameNotFound;

    const bool faceDown = cmd.face_down();
    if (faceDown == card->getFaceDown())
        return Response::RespContextError;

    card->setFaceDown(faceDown);

    Event_FlipCard event;
    event.set_zone_name(zone->getName().toStdString());
    event.set_card_id(card->getId());
    if (!faceDown)
        event.set_card_name(card->getName().toStdString());
    event.set_face_down(faceDown);
    ges.enqueueGameEvent(event, playerId);

    QString ptString = QString::fromStdString(cmd.pt());
    if (!ptString.isEmpty() && !faceDown)
        setCardAttrHelper(ges, playerId, zone->getName(), card->getId(), AttrPT, ptString);

    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdAttachCard(const Command_AttachCard &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator)
        return Response::RespFunctionNotAllowed;

    if (!game->getGameStarted())
        return Response::RespGameNotStarted;
    if (conceded)
        return Response::RespContextError;

    Server_CardZone *startzone = zones.value(QString::fromStdString(cmd.start_zone()));
    if (!startzone)
        return Response::RespNameNotFound;

    Server_Card *card = startzone->getCard(cmd.card_id());
    if (!card)
        return Response::RespNameNotFound;

    Server_Player *targetPlayer = nullptr;
    Server_CardZone *targetzone = nullptr;
    Server_Card *targetCard = nullptr;

    if (cmd.has_target_player_id()) {
        targetPlayer = game->getPlayers().value(cmd.target_player_id());
        if (!targetPlayer)
            return Response::RespNameNotFound;
    } else if (!card->getParentCard())
        return Response::RespContextError;
    if (targetPlayer)
        targetzone = targetPlayer->getZones().value(QString::fromStdString(cmd.target_zone()));
    if (targetzone) {
        // This is currently enough to make sure cards don't get attached to a card that is not on the table.
        // Possibly a flag will have to be introduced for this sometime.
        if (!targetzone->hasCoords())
            return Response::RespContextError;
        if (cmd.has_target_card_id())
            targetCard = targetzone->getCard(cmd.target_card_id());
        if (targetCard) {
            if (targetCard->getParentCard())
                return Response::RespContextError;
        } else
            return Response::RespNameNotFound;
    }
    if (!startzone->hasCoords())
        return Response::RespContextError;

    // Get all arrows pointing to or originating from the card being attached and delete them.
    QMapIterator<int, Server_Player *> playerIterator(game->getPlayers());
    while (playerIterator.hasNext()) {
        Server_Player *p = playerIterator.next().value();
        QList<Server_Arrow *> arrows = p->getArrows().values();
        QList<Server_Arrow *> toDelete;
        for (auto a : arrows) {
            auto *tCard = qobject_cast<Server_Card *>(a->getTargetItem());
            if ((tCard == card) || (a->getStartCard() == card))
                toDelete.append(a);
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
        for (auto &i : attachedList)
            i->getZone()->getPlayer()->unattachCard(ges, i);

        card->setParentCard(targetCard);
        const int oldX = card->getX();
        card->setCoords(-1, card->getY());
        startzone->updateCardCoordinates(card, oldX, card->getY());

        if (targetzone->isColumnStacked(targetCard->getX(), targetCard->getY())) {
            auto *cardToMove = new CardToMove;
            cardToMove->set_card_id(targetCard->getId());
            targetPlayer->moveCard(ges, targetzone, QList<const CardToMove *>() << cardToMove, targetzone,
                                   targetzone->getFreeGridColumn(-2, targetCard->getY(), targetCard->getName(), false),
                                   targetCard->getY(), targetCard->getFaceDown());
            delete cardToMove;
        }

        Event_AttachCard event;
        event.set_start_zone(startzone->getName().toStdString());
        event.set_card_id(card->getId());
        event.set_target_player_id(targetPlayer->getPlayerId());
        event.set_target_zone(targetzone->getName().toStdString());
        event.set_target_card_id(targetCard->getId());
        ges.enqueueGameEvent(event, playerId);

        startzone->fixFreeSpaces(ges);
    } else
        unattachCard(ges, card);

    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdCreateToken(const Command_CreateToken &cmd, ResponseContainer &rc, GameEventStorage &ges)
{
    if (spectator)
        return Response::RespFunctionNotAllowed;

    if (!game->getGameStarted())
        return Response::RespGameNotStarted;
    if (conceded)
        return Response::RespContextError;

    Server_CardZone *zone = zones.value(QString::fromStdString(cmd.zone()));
    if (!zone)
        return Response::RespNameNotFound;

    QString cardName = QString::fromStdString(cmd.card_name());
    int x = cmd.x();
    int y = cmd.y();
    if (zone->hasCoords())
        x = zone->getFreeGridColumn(x, y, cardName, false);
    if (x < 0)
        x = 0;
    if (y < 0)
        y = 0;

    Server_Card *card = new Server_Card(cardName, newCardId(), x, y);
    card->moveToThread(thread());
    card->setPT(QString::fromStdString(cmd.pt()));
    card->setColor(QString::fromStdString(cmd.color()));
    card->setAnnotation(QString::fromStdString(cmd.annotation()));
    card->setDestroyOnZoneChange(cmd.destroy_on_zone_change());

    zone->insertCard(card, x, y);

    Event_CreateToken event;
    event.set_zone_name(zone->getName().toStdString());
    event.set_card_id(card->getId());
    event.set_card_name(card->getName().toStdString());
    event.set_color(card->getColor().toStdString());
    event.set_pt(card->getPT().toStdString());
    event.set_annotation(card->getAnnotation().toStdString());
    event.set_destroy_on_zone_change(card->getDestroyOnZoneChange());
    event.set_x(x);
    event.set_y(y);
    ges.enqueueGameEvent(event, playerId);

    // check if the token is a replacement for an existing card
    if (cmd.target_card_id() < 0)
        return Response::RespOk;

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
    if (spectator)
        return Response::RespFunctionNotAllowed;

    if (!game->getGameStarted())
        return Response::RespGameNotStarted;
    if (conceded)
        return Response::RespContextError;

    Server_Player *startPlayer = game->getPlayers().value(cmd.start_player_id());
    Server_Player *targetPlayer = game->getPlayers().value(cmd.target_player_id());
    if (!startPlayer || !targetPlayer)
        return Response::RespNameNotFound;
    QString startZoneName = QString::fromStdString(cmd.start_zone());
    Server_CardZone *startZone = startPlayer->getZones().value(startZoneName);
    bool playerTarget = !cmd.has_target_zone();
    Server_CardZone *targetZone = nullptr;
    if (!playerTarget)
        targetZone = targetPlayer->getZones().value(QString::fromStdString(cmd.target_zone()));
    if (!startZone || (!targetZone && !playerTarget))
        return Response::RespNameNotFound;
    if (startZone->getType() != ServerInfo_Zone::PublicZone)
        return Response::RespContextError;
    Server_Card *startCard = startZone->getCard(cmd.start_card_id());
    if (!startCard)
        return Response::RespNameNotFound;
    Server_Card *targetCard = nullptr;
    if (!playerTarget) {
        if (targetZone->getType() != ServerInfo_Zone::PublicZone)
            return Response::RespContextError;
        targetCard = targetZone->getCard(cmd.target_card_id());
    }

    Server_ArrowTarget *targetItem;
    if (playerTarget)
        targetItem = targetPlayer;
    else
        targetItem = targetCard;
    if (!targetItem)
        return Response::RespNameNotFound;

    QMapIterator<int, Server_Arrow *> arrowIterator(arrows);
    while (arrowIterator.hasNext()) {
        Server_Arrow *temp = arrowIterator.next().value();
        if ((temp->getStartCard() == startCard) && (temp->getTargetItem() == targetItem))
            return Response::RespContextError;
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
    if (spectator)
        return Response::RespFunctionNotAllowed;

    if (!game->getGameStarted())
        return Response::RespGameNotStarted;
    if (conceded)
        return Response::RespContextError;

    if (!deleteArrow(cmd.arrow_id()))
        return Response::RespNameNotFound;

    Event_DeleteArrow event;
    event.set_arrow_id(cmd.arrow_id());
    ges.enqueueGameEvent(event, playerId);

    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdSetCardAttr(const Command_SetCardAttr &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator)
        return Response::RespFunctionNotAllowed;

    if (!game->getGameStarted())
        return Response::RespGameNotStarted;
    if (conceded)
        return Response::RespContextError;

    return setCardAttrHelper(ges, playerId, QString::fromStdString(cmd.zone()), cmd.card_id(), cmd.attribute(),
                             QString::fromStdString(cmd.attr_value()));
}

Response::ResponseCode
Server_Player::cmdSetCardCounter(const Command_SetCardCounter &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator)
        return Response::RespFunctionNotAllowed;

    if (!game->getGameStarted())
        return Response::RespGameNotStarted;
    if (conceded)
        return Response::RespContextError;

    Server_CardZone *zone = zones.value(QString::fromStdString(cmd.zone()));
    if (!zone)
        return Response::RespNameNotFound;
    if (!zone->hasCoords())
        return Response::RespContextError;

    Server_Card *card = zone->getCard(cmd.card_id());
    if (!card)
        return Response::RespNameNotFound;

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
    if (spectator)
        return Response::RespFunctionNotAllowed;

    if (!game->getGameStarted())
        return Response::RespGameNotStarted;
    if (conceded)
        return Response::RespContextError;

    Server_CardZone *zone = zones.value(QString::fromStdString(cmd.zone()));
    if (!zone)
        return Response::RespNameNotFound;
    if (!zone->hasCoords())
        return Response::RespContextError;

    Server_Card *card = zone->getCard(cmd.card_id());
    if (!card)
        return Response::RespNameNotFound;

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
    if (spectator)
        return Response::RespFunctionNotAllowed;

    if (!game->getGameStarted())
        return Response::RespGameNotStarted;
    if (conceded)
        return Response::RespContextError;

    Server_Counter *c = counters.value(cmd.counter_id(), 0);
    if (!c)
        return Response::RespNameNotFound;

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
    if (spectator)
        return Response::RespFunctionNotAllowed;

    if (!game->getGameStarted())
        return Response::RespGameNotStarted;
    if (conceded)
        return Response::RespContextError;

    Server_Counter *c = new Server_Counter(newCounterId(), QString::fromStdString(cmd.counter_name()),
                                           cmd.counter_color(), cmd.radius(), cmd.value());
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
    if (spectator)
        return Response::RespFunctionNotAllowed;

    if (!game->getGameStarted())
        return Response::RespGameNotStarted;
    if (conceded)
        return Response::RespContextError;

    Server_Counter *c = counters.value(cmd.counter_id(), 0);
    ;
    if (!c)
        return Response::RespNameNotFound;

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
    if (spectator)
        return Response::RespFunctionNotAllowed;

    if (!game->getGameStarted())
        return Response::RespGameNotStarted;
    if (conceded)
        return Response::RespContextError;

    Server_Counter *counter = counters.value(cmd.counter_id(), 0);
    if (!counter)
        return Response::RespNameNotFound;
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
    if (spectator)
        return Response::RespFunctionNotAllowed;

    if (!game->getGameStarted())
        return Response::RespGameNotStarted;
    if (conceded)
        return Response::RespContextError;

    game->nextTurn();
    return Response::RespOk;
}

Response::ResponseCode Server_Player::cmdSetActivePhase(const Command_SetActivePhase &cmd,
                                                        ResponseContainer & /*rc*/,
                                                        GameEventStorage & /*ges*/)
{
    if (spectator)
        return Response::RespFunctionNotAllowed;

    if (!game->getGameStarted())
        return Response::RespGameNotStarted;
    if (conceded)
        return Response::RespContextError;

    if (game->getActivePlayer() != playerId)
        return Response::RespContextError;
    game->setActivePhase(cmd.phase());

    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdDumpZone(const Command_DumpZone &cmd, ResponseContainer &rc, GameEventStorage &ges)
{
    if (!game->getGameStarted())
        return Response::RespGameNotStarted;

    Server_Player *otherPlayer = game->getPlayers().value(cmd.player_id());
    if (!otherPlayer)
        return Response::RespNameNotFound;
    Server_CardZone *zone = otherPlayer->getZones().value(QString::fromStdString(cmd.zone_name()));
    if (!zone)
        return Response::RespNameNotFound;
    if (!((zone->getType() == ServerInfo_Zone::PublicZone) || (this == otherPlayer)))
        return Response::RespContextError;

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
        if (zone->getType() == ServerInfo_Zone::HiddenZone)
            cardInfo->set_id(i);
        else {
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
    if (zone->getType() == ServerInfo_Zone::HiddenZone) {
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
Server_Player::cmdStopDumpZone(const Command_StopDumpZone &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (!game->getGameStarted())
        return Response::RespGameNotStarted;
    if (conceded)
        return Response::RespContextError;

    Server_Player *otherPlayer = game->getPlayers().value(cmd.player_id());
    if (!otherPlayer)
        return Response::RespNameNotFound;
    Server_CardZone *zone = otherPlayer->getZones().value(QString::fromStdString(cmd.zone_name()));
    if (!zone)
        return Response::RespNameNotFound;

    if (zone->getType() == ServerInfo_Zone::HiddenZone) {
        zone->setCardsBeingLookedAt(0);

        Event_StopDumpZone event;
        event.set_zone_owner_id(cmd.player_id());
        event.set_zone_name(zone->getName().toStdString());
        ges.enqueueGameEvent(event, playerId);
    }
    return Response::RespOk;
}

Response::ResponseCode
Server_Player::cmdRevealCards(const Command_RevealCards &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator)
        return Response::RespFunctionNotAllowed;

    if (!game->getGameStarted())
        return Response::RespGameNotStarted;
    if (conceded)
        return Response::RespContextError;

    if (cmd.has_player_id()) {
        Server_Player *otherPlayer = game->getPlayers().value(cmd.player_id());
        if (!otherPlayer)
            return Response::RespNameNotFound;
    }
    Server_CardZone *zone = zones.value(QString::fromStdString(cmd.zone_name()));
    if (!zone)
        return Response::RespNameNotFound;

    QList<Server_Card *> cardsToReveal;
    if (cmd.top_cards() != -1) {
        for (int i = 0; i < cmd.top_cards(); i++) {
            Server_Card *card = zone->getCard(i);
            if (!card)
                return Response::RespNameNotFound;
            cardsToReveal.append(card);
        }
    } else if (!cmd.has_card_id())
        cardsToReveal = zone->getCards();
    else if (cmd.card_id() == -2) {
        if (zone->getCards().isEmpty())
            return Response::RespContextError;
        cardsToReveal.append(zone->getCards().at(rng->rand(0, zone->getCards().size() - 1)));
    } else {
        Server_Card *card = zone->getCard(cmd.card_id());
        if (!card)
            return Response::RespNameNotFound;
        cardsToReveal.append(card);
    }

    Event_RevealCards eventOthers;
    eventOthers.set_grant_write_access(cmd.grant_write_access());
    eventOthers.set_zone_name(zone->getName().toStdString());
    eventOthers.set_number_of_cards(cardsToReveal.size());
    if (cmd.has_card_id())
        eventOthers.set_card_id(cmd.card_id());
    if (cmd.has_player_id())
        eventOthers.set_other_player_id(cmd.player_id());

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
        if (cmd.grant_write_access())
            zone->addWritePermission(cmd.player_id());

        ges.enqueueGameEvent(eventPrivate, playerId, GameEventStorageItem::SendToPrivate, cmd.player_id());
        ges.enqueueGameEvent(eventOthers, playerId, GameEventStorageItem::SendToOthers);
    } else {
        if (cmd.grant_write_access()) {
            const QList<int> &playerIds = game->getPlayers().keys();
            for (int playerId : playerIds)
                zone->addWritePermission(playerId);
        }

        ges.enqueueGameEvent(eventPrivate, playerId);
    }

    return Response::RespOk;
}

Response::ResponseCode Server_Player::cmdChangeZoneProperties(const Command_ChangeZoneProperties &cmd,
                                                              ResponseContainer & /* rc */,
                                                              GameEventStorage &ges)
{
    Server_CardZone *zone = zones.value(QString::fromStdString(cmd.zone_name()));
    if (!zone)
        return Response::RespNameNotFound;

    Event_ChangeZoneProperties event;
    event.set_zone_name(cmd.zone_name());

    if (cmd.has_always_reveal_top_card()) {
        if (zone->getAlwaysRevealTopCard() == cmd.always_reveal_top_card())
            return Response::RespContextError;
        zone->setAlwaysRevealTopCard(cmd.always_reveal_top_card());
        event.set_always_reveal_top_card(cmd.always_reveal_top_card());

        ges.enqueueGameEvent(event, playerId);

        if (!zone->getCards().isEmpty() && cmd.always_reveal_top_card()) {
            Event_RevealCards revealEvent;
            revealEvent.set_zone_name(zone->getName().toStdString());
            revealEvent.set_card_id(0);
            zone->getCards().first()->getInfo(revealEvent.add_cards());

            ges.enqueueGameEvent(revealEvent, playerId);
        }
        return Response::RespOk;
    } else
        return Response::RespContextError;
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
        case GameCommand::STOP_DUMP_ZONE:
            return cmdStopDumpZone(command.GetExtension(Command_StopDumpZone::ext), rc, ges);
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

        default:
            return Response::RespInvalidCommand;
    }
}

void Server_Player::sendGameEvent(const GameEventContainer &cont)
{
    QMutexLocker locker(&playerMutex);

    if (userInterface)
        userInterface->sendProtocolItem(cont);
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
    if (!(userInfo->user_level() & ServerInfo_User::IsRegistered) || spectator)
        game->removePlayer(this, Event_Leave::USER_DISCONNECTED);
    else
        setUserInterface(nullptr);
}

void Server_Player::getInfo(ServerInfo_Player *info,
                            Server_Player *playerWhosAsking,
                            bool omniscient,
                            bool withUserInfo)
{
    getProperties(*info->mutable_properties(), withUserInfo);
    if (playerWhosAsking == this)
        if (deck)
            info->set_deck_list(deck->writeToString_Native().toStdString());

    QMapIterator<int, Server_Arrow *> arrowIterator(arrows);
    while (arrowIterator.hasNext())
        arrowIterator.next().value()->getInfo(info->add_arrow_list());

    QMapIterator<int, Server_Counter *> counterIterator(counters);
    while (counterIterator.hasNext())
        counterIterator.next().value()->getInfo(info->add_counter_list());

    QMapIterator<QString, Server_CardZone *> zoneIterator(zones);
    while (zoneIterator.hasNext())
        zoneIterator.next().value()->getInfo(info->add_zone_list(), playerWhosAsking, omniscient);
}
