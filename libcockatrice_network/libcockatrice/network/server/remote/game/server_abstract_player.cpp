#include "server_abstract_player.h"

#include "server_arrow.h"
#include "server_card.h"
#include "server_cardzone.h"
#include "server_game.h"
#include "server_move_card_struct.h"

#include <QDebug>
#include <QRegularExpression>
#include <algorithm>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/protocol/pb/command_attach_card.pb.h>
#include <libcockatrice/protocol/pb/command_change_zone_properties.pb.h>
#include <libcockatrice/protocol/pb/command_create_arrow.pb.h>
#include <libcockatrice/protocol/pb/command_create_token.pb.h>
#include <libcockatrice/protocol/pb/command_delete_arrow.pb.h>
#include <libcockatrice/protocol/pb/command_dump_zone.pb.h>
#include <libcockatrice/protocol/pb/command_flip_card.pb.h>
#include <libcockatrice/protocol/pb/command_inc_card_counter.pb.h>
#include <libcockatrice/protocol/pb/command_move_card.pb.h>
#include <libcockatrice/protocol/pb/command_ready_start.pb.h>
#include <libcockatrice/protocol/pb/command_reveal_cards.pb.h>
#include <libcockatrice/protocol/pb/command_roll_die.pb.h>
#include <libcockatrice/protocol/pb/command_set_card_attr.pb.h>
#include <libcockatrice/protocol/pb/command_set_card_counter.pb.h>
#include <libcockatrice/protocol/pb/context_concede.pb.h>
#include <libcockatrice/protocol/pb/context_move_card.pb.h>
#include <libcockatrice/protocol/pb/context_ready_start.pb.h>
#include <libcockatrice/protocol/pb/context_undo_draw.pb.h>
#include <libcockatrice/protocol/pb/event_attach_card.pb.h>
#include <libcockatrice/protocol/pb/event_change_zone_properties.pb.h>
#include <libcockatrice/protocol/pb/event_create_arrow.pb.h>
#include <libcockatrice/protocol/pb/event_create_token.pb.h>
#include <libcockatrice/protocol/pb/event_delete_arrow.pb.h>
#include <libcockatrice/protocol/pb/event_destroy_card.pb.h>
#include <libcockatrice/protocol/pb/event_dump_zone.pb.h>
#include <libcockatrice/protocol/pb/event_flip_card.pb.h>
#include <libcockatrice/protocol/pb/event_move_card.pb.h>
#include <libcockatrice/protocol/pb/event_player_properties_changed.pb.h>
#include <libcockatrice/protocol/pb/event_reveal_cards.pb.h>
#include <libcockatrice/protocol/pb/event_roll_die.pb.h>
#include <libcockatrice/protocol/pb/event_set_card_attr.pb.h>
#include <libcockatrice/protocol/pb/event_set_card_counter.pb.h>
#include <libcockatrice/protocol/pb/response.pb.h>
#include <libcockatrice/protocol/pb/response_dump_zone.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_player.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_user.pb.h>
#include <libcockatrice/rng/rng_abstract.h>
#include <libcockatrice/utility/trice_limits.h>

Server_AbstractPlayer::Server_AbstractPlayer(Server_Game *_game,
                                             int _playerId,
                                             const ServerInfo_User &_userInfo,
                                             bool _judge,
                                             Server_AbstractUserInterface *_userInterface)
    : Server_AbstractParticipant(_game, _playerId, _userInfo, _judge, _userInterface), conceded(false), deck(nullptr),
      sideboardLocked(true), readyStart(false), nextCardId(0)
{
    spectator = false;
}

Server_AbstractPlayer::~Server_AbstractPlayer() = default;

void Server_AbstractPlayer::prepareDestroy()
{
    delete deck;
    deck = nullptr;

    removeFromGame();
    clearZones();

    deleteLater();
}

int Server_AbstractPlayer::newCardId()
{
    return nextCardId++;
}

int Server_AbstractPlayer::newArrowId() const
{
    int id = 0;
    for (Server_Arrow *a : arrows) {
        if (a->getId() > id) {
            id = a->getId();
        }
    }
    return id + 1;
}

void Server_AbstractPlayer::setupZones()
{
    nextCardId = 0;
}

void Server_AbstractPlayer::clearZones()
{
    for (Server_CardZone *zone : zones) {
        delete zone;
    }
    zones.clear();

    for (Server_Arrow *arrow : arrows) {
        delete arrow;
    }
    arrows.clear();
}

void Server_AbstractPlayer::addZone(Server_CardZone *zone)
{
    zones.insert(zone->getName(), zone);
}

void Server_AbstractPlayer::addArrow(Server_Arrow *arrow)
{
    arrows.insert(arrow->getId(), arrow);
}

void Server_AbstractPlayer::updateArrowId(int id)
{
    auto *arrow = arrows.take(id);
    arrows.insert(arrow->getId(), arrow);
}

bool Server_AbstractPlayer::deleteArrow(int arrowId)
{
    Server_Arrow *arrow = arrows.value(arrowId, 0);
    if (!arrow) {
        return false;
    }
    arrows.remove(arrowId);
    delete arrow;
    return true;
}

/**
 * Creates the create token event.
 * By default, will set event's name and color fields to empty if the token is face-down
 */
static Event_CreateToken
makeCreateTokenEvent(Server_CardZone *zone, Server_Card *card, int xCoord, int yCoord, bool revealFacedownInfo = false)
{
    Event_CreateToken event;
    event.set_zone_name(zone->getName().toStdString());
    event.set_card_id(card->getId());
    event.set_face_down(card->getFaceDown());

    if (!card->getFaceDown() || revealFacedownInfo) {
        event.set_card_name(card->getName().toStdString());
        event.set_card_provider_id(card->getProviderId().toStdString());
    }

    event.set_color(card->getColor().toStdString());
    event.set_pt(card->getPT().toStdString());
    event.set_annotation(card->getAnnotation().toStdString());
    event.set_destroy_on_zone_change(card->getDestroyOnZoneChange());
    event.set_x(xCoord);
    event.set_y(yCoord);
    return event;
}

static Event_AttachCard makeAttachCardEvent(Server_Card *attachedCard, Server_Card *parentCard = nullptr)
{
    Event_AttachCard event;
    event.set_start_zone(attachedCard->getZone()->getName().toStdString());
    event.set_card_id(attachedCard->getId());

    if (parentCard) {
        event.set_target_player_id(parentCard->getZone()->getPlayer()->getPlayerId());
        event.set_target_zone(parentCard->getZone()->getName().toStdString());
        event.set_target_card_id(parentCard->getId());
    }

    return event;
}

/**
 * Determines whether moving the card from startZone to targetZone should cause the card to be destroyed.
 */
static bool
shouldDestroyOnMove(const Server_Card *card, const Server_CardZone *startZone, const Server_CardZone *targetZone)
{
    if (!card->getDestroyOnZoneChange()) {
        return false;
    }

    if (startZone->getName() == targetZone->getName()) {
        return false;
    }

    // Allow tokens on the stack
    if ((startZone->getName() == "table" || startZone->getName() == "stack") &&
        (targetZone->getName() == "table" || targetZone->getName() == "stack")) {
        return false;
    }

    return true;
}

Response::ResponseCode Server_AbstractPlayer::moveCard(GameEventStorage &ges,
                                                       Server_CardZone *startzone,
                                                       const QList<const CardToMove *> &_cards,
                                                       Server_CardZone *targetzone,
                                                       int xCoord,
                                                       int yCoord,
                                                       bool fixFreeSpaces,
                                                       bool undoingDraw,
                                                       bool isReversed)
{
    // Disallow controller change to other zones than the table.
    if (((targetzone->getType() != ServerInfo_Zone::PublicZone) || !targetzone->hasCoords()) &&
        (startzone->getPlayer() != targetzone->getPlayer()) && !judge) {
        return Response::RespContextError;
    }

    if (!targetzone->hasCoords() && (xCoord <= -1)) {
        xCoord = targetzone->getCards().size();
    }

    std::set<MoveCardStruct> cardsToMove;
    QSet<int> cardIdsToMove;
    for (auto _card : _cards) {
        // The same card being moved twice would lead to undefined behaviour.
        if (cardIdsToMove.contains(_card->card_id())) {
            continue;
        }
        cardIdsToMove.insert(_card->card_id());

        // Consistency checks. In case the command contains illegal moves, try to resolve the legal ones still.
        int position;
        Server_Card *card = startzone->getCard(_card->card_id(), &position);
        if (!card) {
            return Response::RespNameNotFound;
        }

        // do not allow attached cards to move around on the table
        if (card->getParentCard() && targetzone->getName() == "table") {
            continue;
        }

        // do not allow cards with attachments to stack with other cards
        if (!card->getAttachedCards().isEmpty() && !targetzone->isColumnEmpty(xCoord, yCoord)) {
            continue;
        }

        cardsToMove.insert(MoveCardStruct{card, position, _card});
    }
    // In case all moves were filtered out, abort.
    if (cardsToMove.empty()) {
        return Response::RespContextError;
    }

    int xIndex = -1;
    bool revealTopStart = false;
    bool revealTopTarget = false;

    for (auto cardStruct : cardsToMove) {
        Server_Card *card = cardStruct.card;
        const CardToMove *thisCardProperties = cardStruct.cardToMove;
        int originalPosition = cardStruct.position;
        bool faceDown = targetzone->hasCoords() &&
                        (thisCardProperties->has_face_down() ? thisCardProperties->face_down() : card->getFaceDown());

        bool sourceBeingLookedAt;
        int position = startzone->removeCard(card, sourceBeingLookedAt);

        // Attachment relationships can be retained when moving a card onto the opponent's table
        if (startzone->getName() != targetzone->getName()) {
            // Delete all attachment relationships
            if (card->getParentCard()) {
                card->setParentCard(nullptr);
            }

            // Make a copy of the list because the original one gets modified during the loop
            QList<Server_Card *> attachedCards = card->getAttachedCards();
            for (auto &attachedCard : attachedCards) {
                attachedCard->getZone()->getPlayer()->unattachCard(ges, attachedCard);
            }
        }

        if (startzone != targetzone) {
            // Delete all arrows from and to the card
            for (auto *player : game->getPlayers().values()) {
                QList<int> arrowsToDelete;
                for (Server_Arrow *arrow : player->getArrows()) {
                    if ((arrow->getStartCard() == card) || (arrow->getTargetItem() == card))
                        arrowsToDelete.append(arrow->getId());
                }
                for (int j : arrowsToDelete) {
                    player->deleteArrow(j);
                }
            }
        }

        if (shouldDestroyOnMove(card, startzone, targetzone)) {
            Event_DestroyCard event;
            event.set_zone_name(startzone->getName().toStdString());
            event.set_card_id(static_cast<google::protobuf::uint32>(card->getId()));
            ges.enqueueGameEvent(event, playerId);

            if (Server_Card *stashedCard = card->takeStashedCard()) {
                stashedCard->setId(newCardId());
                ges.enqueueGameEvent(makeCreateTokenEvent(startzone, stashedCard, card->getX(), card->getY()),
                                     playerId);
                card->deleteLater();
                card = stashedCard;
            } else {
                card->deleteLater();
                card = nullptr;
            }
        }

        if (card) {
            ++xIndex;
            int newX = isReversed ? targetzone->getCards().size() - xCoord + xIndex : xCoord + xIndex;

            if (targetzone->hasCoords()) {
                newX = targetzone->getFreeGridColumn(newX, yCoord, card->getName(), faceDown);
            } else {
                yCoord = 0;
                card->resetState(targetzone->getName() == "stack");
            }

            targetzone->insertCard(card, newX, yCoord);
            int targetLookedCards = targetzone->getCardsBeingLookedAt();
            bool sourceKnownToPlayer = isReversed || (sourceBeingLookedAt && !card->getFaceDown());
            if (targetzone->getType() == ServerInfo_Zone::HiddenZone && targetLookedCards >= newX) {
                if (sourceKnownToPlayer) {
                    targetLookedCards += 1;
                } else {
                    targetLookedCards = newX;
                }
                targetzone->setCardsBeingLookedAt(targetLookedCards);
            }

            bool targetHiddenToOthers = faceDown || (targetzone->getType() != ServerInfo_Zone::PublicZone);
            bool sourceHiddenToOthers = card->getFaceDown() || (startzone->getType() != ServerInfo_Zone::PublicZone);

            int oldCardId = card->getId();
            if ((faceDown && (startzone != targetzone)) || (targetzone->getPlayer() != startzone->getPlayer())) {
                card->setId(targetzone->getPlayer()->newCardId());
            }
            card->setFaceDown(faceDown);

            Event_MoveCard eventOthers;
            eventOthers.set_start_player_id(startzone->getPlayer()->getPlayerId());
            eventOthers.set_start_zone(startzone->getName().toStdString());
            eventOthers.set_target_player_id(targetzone->getPlayer()->getPlayerId());
            if (startzone != targetzone) {
                eventOthers.set_target_zone(targetzone->getName().toStdString());
            }
            eventOthers.set_y(yCoord);
            eventOthers.set_face_down(faceDown);

            Event_MoveCard eventPrivate(eventOthers);
            if (sourceBeingLookedAt || targetzone->getType() != ServerInfo_Zone::HiddenZone ||
                startzone->getType() != ServerInfo_Zone::HiddenZone) {
                eventPrivate.set_card_id(oldCardId);
                eventPrivate.set_new_card_id(card->getId());
            } else {
                eventPrivate.set_card_id(-1);
                eventPrivate.set_new_card_id(-1);
            }
            if (sourceKnownToPlayer || !(faceDown || targetzone->getType() == ServerInfo_Zone::HiddenZone)) {
                QString privateCardName = card->getName();
                eventPrivate.set_card_name(privateCardName.toStdString());
                eventPrivate.set_new_card_provider_id(card->getProviderId().toStdString());
            }
            if (startzone->getType() == ServerInfo_Zone::HiddenZone) {
                eventPrivate.set_position(position);
            } else {
                eventPrivate.set_position(-1);
            }

            eventPrivate.set_x(newX);

            if (
                // cards from public zones have their id known, their previous position is already known, the event does
                // not accomodate for previous locations in zones with coordinates (which are always public)
                (startzone->getType() != ServerInfo_Zone::PublicZone) &&
                // other players are not allowed to be able to track which card is which in private zones like the hand
                (startzone->getType() != ServerInfo_Zone::PrivateZone)) {
                eventOthers.set_position(position);
            }
            if (
                // other players are not allowed to be able to track which card is which in private zones like the hand
                (targetzone->getType() != ServerInfo_Zone::PrivateZone)) {
                eventOthers.set_x(newX);
            }

            if ((startzone->getType() == ServerInfo_Zone::PublicZone) ||
                (targetzone->getType() == ServerInfo_Zone::PublicZone)) {
                eventOthers.set_card_id(oldCardId);
                if (!(sourceHiddenToOthers && targetHiddenToOthers)) {
                    QString publicCardName = card->getName();
                    eventOthers.set_card_name(publicCardName.toStdString());
                    eventOthers.set_new_card_provider_id(card->getProviderId().toStdString());
                }
                eventOthers.set_new_card_id(card->getId());
            }

            ges.enqueueGameEvent(eventPrivate, playerId, GameEventStorageItem::SendToPrivate, playerId);
            ges.enqueueGameEvent(eventOthers, playerId, GameEventStorageItem::SendToOthers);

            if (originalPosition == 0) {
                revealTopStart = true;
            }
            if (newX == 0) {
                revealTopTarget = true;
            }

            // handle side effects for this card
            onCardBeingMoved(ges, cardStruct, startzone, targetzone, undoingDraw);
        }
    }
    if (revealTopStart) {
        revealTopCardIfNeeded(startzone, ges);
    }
    if (targetzone != startzone && revealTopTarget) {
        revealTopCardIfNeeded(targetzone, ges);
    }
    if (undoingDraw) {
        ges.setGameEventContext(Context_UndoDraw());
    } else {
        ges.setGameEventContext(Context_MoveCard());
    }

    if (startzone->hasCoords() && fixFreeSpaces) {
        startzone->fixFreeSpaces(ges);
    }

    return Response::RespOk;
}

void Server_AbstractPlayer::onCardBeingMoved(GameEventStorage &ges,
                                             const MoveCardStruct &cardStruct,
                                             Server_CardZone *startzone,
                                             Server_CardZone *targetzone,
                                             bool /*undoingDraw*/)
{
    Server_Card *card = cardStruct.card;
    const CardToMove *thisCardProperties = cardStruct.cardToMove;

    // set card to be tapped
    if (thisCardProperties->tapped()) {
        setCardAttrHelper(ges, targetzone->getPlayer()->getPlayerId(), targetzone->getName(), card->getId(), AttrTapped,
                          "1");
    }

    // set card pt
    QString ptString = QString::fromStdString(thisCardProperties->pt());
    if (!ptString.isEmpty()) {
        setCardAttrHelper(ges, targetzone->getPlayer()->getPlayerId(), targetzone->getName(), card->getId(), AttrPT,
                          ptString);
    }

    // If card is transferring to a different player, leave an annotation of who actually "owns" the card
    const auto &priorAnnotation = card->getAnnotation();
    if (startzone->getPlayer() != targetzone->getPlayer() && !priorAnnotation.contains("Owner:")) {
        const auto &ownerAnnotation = "Owner: " + QString::fromStdString(startzone->getPlayer()->getUserInfo()->name());
        const auto &newAnnotation =
            priorAnnotation.isEmpty() ? ownerAnnotation : ownerAnnotation + "\n\n" + priorAnnotation;
        setCardAttrHelper(ges, targetzone->getPlayer()->getPlayerId(), targetzone->getName(), card->getId(),
                          AttrAnnotation, newAnnotation, card);
    }
}

void Server_AbstractPlayer::revealTopCardIfNeeded(Server_CardZone *zone, GameEventStorage &ges)
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

void Server_AbstractPlayer::unattachCard(GameEventStorage &ges, Server_Card *card)
{
    Server_CardZone *zone = card->getZone();
    Server_Card *parentCard = card->getParentCard();
    card->setParentCard(nullptr);

    ges.enqueueGameEvent(makeAttachCardEvent(card), playerId);

    auto *cardToMove = new CardToMove;
    cardToMove->set_card_id(card->getId());
    moveCard(ges, zone, QList<const CardToMove *>() << cardToMove, zone, -1, card->getY(), card->getFaceDown());
    delete cardToMove;

    if (parentCard->getZone()) {
        parentCard->getZone()->updateCardCoordinates(parentCard, parentCard->getX(), parentCard->getY());
    }
}

Response::ResponseCode Server_AbstractPlayer::setCardAttrHelper(GameEventStorage &ges,
                                                                int targetPlayerId,
                                                                const QString &zoneName,
                                                                int cardId,
                                                                CardAttribute attribute,
                                                                const QString &attrValue,
                                                                Server_Card *unzonedCard)
{
    Server_CardZone *zone = getZones().value(zoneName);
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
        Server_Card *card = unzonedCard == nullptr ? zone->getCard(cardId) : unzonedCard;
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
Server_AbstractPlayer::cmdConcede(const Command_Concede & /*cmd*/, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }
    if (conceded) {
        return Response::RespContextError;
    }

    setConceded(true);
    game->removeArrowsRelatedToPlayer(ges, this);
    game->unattachCards(ges, this);

    playerMutex.lock();

    // Return cards to their rightful owners before conceding the game
    static const QRegularExpression ownerRegex{"Owner: ?([^\n]+)"};
    for (const auto &card : zones.value("table")->getCards()) {
        if (card == nullptr) {
            continue;
        }

        const auto &regexResult = ownerRegex.match(card->getAnnotation());
        if (!regexResult.hasMatch()) {
            continue;
        }

        CardToMove cardToMove;
        cardToMove.set_card_id(card->getId());

        for (const auto *player : game->getPlayers()) {
            if (player == nullptr || player->getUserInfo() == nullptr) {
                continue;
            }

            const auto &ownerToReturnTo = regexResult.captured(1);
            const auto &correctOwner = QString::compare(QString::fromStdString(player->getUserInfo()->name()),
                                                        ownerToReturnTo, Qt::CaseInsensitive) == 0;
            if (!correctOwner) {
                continue;
            }

            const auto &startZone = zones.value("table");
            const auto &targetZone = player->getZones().value("table");

            if (startZone == nullptr || targetZone == nullptr) {
                continue;
            }

            moveCard(ges, startZone, QList<const CardToMove *>() << &cardToMove, targetZone, 0, 0, false);
            break;
        }
    }

    playerMutex.unlock();

    // All borrowed cards have been returned, can now continue cleanup process
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

Response::ResponseCode Server_AbstractPlayer::cmdUnconcede(const Command_Unconcede & /*cmd*/,
                                                           ResponseContainer & /*rc*/,
                                                           GameEventStorage &ges)
{
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

Response::ResponseCode
Server_AbstractPlayer::cmdReadyStart(const Command_ReadyStart &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (!deck || game->getGameStarted()) {
        return Response::RespContextError;
    }

    if (readyStart == cmd.ready() && !cmd.force_start()) {
        return Response::RespContextError;
    }

    setReadyStart(cmd.ready());

    Event_PlayerPropertiesChanged event;
    event.mutable_player_properties()->set_ready_start(cmd.ready());
    ges.enqueueGameEvent(event, playerId);
    ges.setGameEventContext(Context_ReadyStart());

    if (cmd.force_start()) {
        if (game->getHostId() != playerId) {
            return Response::RespFunctionNotAllowed;
        }
        game->startGameIfReady(true);
    } else if (cmd.ready()) {
        game->startGameIfReady(false);
    }

    return Response::RespOk;
}

Response::ResponseCode
Server_AbstractPlayer::cmdRollDie(const Command_RollDie &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges) const
{
    if (conceded) {
        return Response::RespContextError;
    }

    const auto validatedSides = static_cast<int>(std::min(std::max(cmd.sides(), MINIMUM_DIE_SIDES), MAXIMUM_DIE_SIDES));
    const auto validatedDiceToRoll =
        static_cast<int>(std::min(std::max(cmd.count(), MINIMUM_DICE_TO_ROLL), MAXIMUM_DICE_TO_ROLL));

    Event_RollDie event;
    event.set_sides(validatedSides);
    for (auto i = 0; i < validatedDiceToRoll; ++i) {
        const auto roll = rng->rand(1, validatedSides);
        if (i == 0) {
            // Backwards compatibility
            event.set_value(roll);
        }
        event.add_values(roll);
    }
    ges.enqueueGameEvent(event, playerId);

    return Response::RespOk;
}

Response::ResponseCode
Server_AbstractPlayer::cmdMoveCard(const Command_MoveCard &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }
    if (conceded) {
        return Response::RespContextError;
    }

    Server_AbstractPlayer *startPlayer = game->getPlayer(cmd.has_start_player_id() ? cmd.start_player_id() : playerId);
    if (!startPlayer) {
        return Response::RespNameNotFound;
    }
    Server_CardZone *startZone = startPlayer->getZones().value(nameFromStdString(cmd.start_zone()));
    if (!startZone) {
        return Response::RespNameNotFound;
    }

    if ((startPlayer != this) && (!startZone->getPlayersWithWritePermission().contains(playerId)) && !judge) {
        return Response::RespContextError;
    }

    Server_AbstractPlayer *targetPlayer = game->getPlayer(cmd.target_player_id());
    if (!targetPlayer) {
        return Response::RespNameNotFound;
    }
    Server_CardZone *targetZone = targetPlayer->getZones().value(nameFromStdString(cmd.target_zone()));
    if (!targetZone) {
        return Response::RespNameNotFound;
    }

    if ((startPlayer != this) && (targetPlayer != this) && !judge) {
        return Response::RespContextError;
    }

    QList<const CardToMove *> cardsToMove;
    for (int i = 0; i < cmd.cards_to_move().card_size(); ++i) {
        cardsToMove.append(&cmd.cards_to_move().card(i));
    }

    return moveCard(ges, startZone, cardsToMove, targetZone, cmd.x(), cmd.y(), true, false, cmd.is_reversed());
}

Response::ResponseCode
Server_AbstractPlayer::cmdFlipCard(const Command_FlipCard &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }
    if (conceded) {
        return Response::RespContextError;
    }

    Server_CardZone *zone = zones.value(nameFromStdString(cmd.zone()));
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
        event.set_card_provider_id(card->getProviderId().toStdString());
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
Server_AbstractPlayer::cmdAttachCard(const Command_AttachCard &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }
    if (conceded) {
        return Response::RespContextError;
    }

    Server_CardZone *startzone = zones.value(nameFromStdString(cmd.start_zone()));
    if (!startzone) {
        return Response::RespNameNotFound;
    }

    Server_Card *card = startzone->getCard(cmd.card_id());
    if (!card) {
        return Response::RespNameNotFound;
    }

    Server_AbstractPlayer *targetPlayer = nullptr;
    Server_CardZone *targetzone = nullptr;
    Server_Card *targetCard = nullptr;

    if (cmd.has_target_player_id()) {
        targetPlayer = game->getPlayer(cmd.target_player_id());
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

    // prevent attaching from non-table zones
    // (attaching from non-table zones is handled client-side by moving the card to table zone first)
    if (!startzone->hasCoords()) {
        return Response::RespContextError;
    }

    for (auto *player : game->getPlayers()) {
        QList<Server_Arrow *> _arrows = player->getArrows().values();
        QList<Server_Arrow *> toDelete;
        for (auto a : _arrows) {
            auto *tCard = qobject_cast<Server_Card *>(a->getTargetItem());
            if ((tCard == card) || (a->getStartCard() == card)) {
                toDelete.append(a);
            }
        }
        for (auto &i : toDelete) {
            Event_DeleteArrow event;
            event.set_arrow_id(i->getId());
            ges.enqueueGameEvent(event, player->getPlayerId());
            player->deleteArrow(i->getId());
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
            auto *cardToMove = new CardToMove;
            cardToMove->set_card_id(targetCard->getId());
            targetPlayer->moveCard(ges, targetzone, QList<const CardToMove *>() << cardToMove, targetzone,
                                   targetzone->getFreeGridColumn(-2, targetCard->getY(), targetCard->getName(), false),
                                   targetCard->getY(), targetCard->getFaceDown());
            delete cardToMove;
        }

        ges.enqueueGameEvent(makeAttachCardEvent(card, targetCard), playerId);

        startzone->fixFreeSpaces(ges);
    } else {
        unattachCard(ges, card);
    }

    return Response::RespOk;
}

Response::ResponseCode
Server_AbstractPlayer::cmdCreateToken(const Command_CreateToken &cmd, ResponseContainer &rc, GameEventStorage &ges)
{
    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }
    if (conceded) {
        return Response::RespContextError;
    }

    Server_CardZone *zone = zones.value(nameFromStdString(cmd.zone()));
    if (!zone) {
        return Response::RespNameNotFound;
    }

    int xCoord = cmd.x();
    int yCoord = cmd.y();

    Server_Card *targetCard = nullptr;
    if (cmd.has_target_card_id()) {
        Server_CardZone *targetZone = zones.value(nameFromStdString(cmd.target_zone()));
        if (targetZone) {
            targetCard = targetZone->getCard(cmd.target_card_id());
            if (targetCard && cmd.target_mode() == Command_CreateToken::TRANSFORM_INTO) {
                if (targetCard->getParentCard()) {
                    ges.enqueueGameEvent(makeAttachCardEvent(targetCard), playerId);
                }

                for (Server_Card *attachedCard : targetCard->getAttachedCards()) {
                    ges.enqueueGameEvent(makeAttachCardEvent(attachedCard),
                                         attachedCard->getZone()->getPlayer()->getPlayerId());
                }

                if (zone->hasCoords() && zone == targetZone) {
                    xCoord = targetCard->getX();
                    yCoord = targetCard->getY();
                }

                targetZone->removeCard(targetCard);

                Event_DestroyCard event;
                event.set_zone_name(targetZone->getName().toStdString());
                event.set_card_id(static_cast<::google::protobuf::uint32>(cmd.target_card_id()));
                ges.enqueueGameEvent(event, playerId);
            }
        }
    }

    const QString cardName = nameFromStdString(cmd.card_name());
    const QString cardProviderId = nameFromStdString(cmd.card_provider_id());
    if (zone->hasCoords()) {
        bool dontStackSameName = cmd.face_down();
        xCoord = zone->getFreeGridColumn(xCoord, yCoord, cardName, dontStackSameName);
    }
    if (xCoord < 0) {
        xCoord = 0;
    }
    if (yCoord < 0) {
        yCoord = 0;
    }

    auto *card = new Server_Card({cardName, cardProviderId}, newCardId(), xCoord, yCoord);
    card->moveToThread(thread());
    // Client should already prevent face-down tokens from having attributes; this just an extra server-side check
    if (!cmd.face_down()) {
        card->setColor(nameFromStdString(cmd.color()));
        card->setPT(nameFromStdString(cmd.pt()));
    }
    card->setAnnotation(nameFromStdString(cmd.annotation()));
    card->setDestroyOnZoneChange(cmd.destroy_on_zone_change());
    card->setFaceDown(cmd.face_down());

    zone->insertCard(card, xCoord, yCoord);
    sendCreateTokenEvents(zone, card, xCoord, yCoord, ges);

    // check if the token is a replacement for an existing card
    if (!targetCard) {
        return Response::RespOk;
    }

    switch (cmd.target_mode()) {
        case Command_CreateToken::ATTACH_TO: {
            Command_AttachCard cmd2;
            cmd2.set_start_zone(cmd.target_zone());
            cmd2.set_card_id(cmd.target_card_id());

            cmd2.set_target_player_id(zone->getPlayer()->getPlayerId());
            cmd2.set_target_zone(cmd.zone());
            cmd2.set_target_card_id(card->getId());

            return cmdAttachCard(cmd2, rc, ges);
        }

        case Command_CreateToken::TRANSFORM_INTO: {
            // Copy attributes that are not present in the CreateToken event
            Event_SetCardAttr event;
            event.set_zone_name(card->getZone()->getName().toStdString());
            event.set_card_id(card->getId());

            if (card->getTapped() != targetCard->getTapped()) {
                card->setAttribute(AttrTapped, QVariant(targetCard->getTapped()).toString(), &event);
                ges.enqueueGameEvent(event, playerId);
            }

            if (card->getAttacking() != targetCard->getAttacking()) {
                card->setAttribute(AttrAttacking, QVariant(targetCard->getAttacking()).toString(), &event);
                ges.enqueueGameEvent(event, playerId);
            }

            if (card->getFaceDown() != targetCard->getFaceDown()) {
                card->setAttribute(AttrFaceDown, QVariant(targetCard->getFaceDown()).toString(), &event);
                ges.enqueueGameEvent(event, playerId);
            }

            if (card->getDoesntUntap() != targetCard->getDoesntUntap()) {
                card->setAttribute(AttrDoesntUntap, QVariant(targetCard->getDoesntUntap()).toString(), &event);
                ges.enqueueGameEvent(event, playerId);
            }

            // Copy counters
            QMapIterator<int, int> i(targetCard->getCounters());
            while (i.hasNext()) {
                i.next();

                Event_SetCardCounter _event;
                _event.set_zone_name(card->getZone()->getName().toStdString());
                _event.set_card_id(card->getId());

                card->setCounter(i.key(), i.value(), &_event);
                ges.enqueueGameEvent(_event, playerId);
            }

            // Copy parent card
            if (Server_Card *parentCard = targetCard->getParentCard()) {
                targetCard->setParentCard(nullptr);
                card->setParentCard(parentCard);

                ges.enqueueGameEvent(makeAttachCardEvent(card, parentCard), playerId);
            }

            // Copy attachments
            while (!targetCard->getAttachedCards().isEmpty()) {
                Server_Card *attachedCard = targetCard->getAttachedCards().last();
                attachedCard->setParentCard(card);

                ges.enqueueGameEvent(makeAttachCardEvent(attachedCard, card),
                                     attachedCard->getZone()->getPlayer()->getPlayerId());
            }

            // Copy Arrows
            for (auto *player : game->getPlayers().values()) {
                QList<int> changedArrowIds;
                for (Server_Arrow *arrow : player->getArrows()) {
                    bool sendGameEvent = false;
                    const auto *startCard = arrow->getStartCard();
                    if (startCard == targetCard) {
                        sendGameEvent = true;
                        arrow->setStartCard(card);
                        startCard = card;
                    }
                    const auto *targetItem = arrow->getTargetItem();
                    if (targetItem == targetCard) {
                        sendGameEvent = true;
                        arrow->setTargetItem(card);
                        targetItem = card;
                    }
                    if (sendGameEvent) {
                        Event_CreateArrow _event;
                        ServerInfo_Arrow *arrowInfo = _event.mutable_arrow_info();
                        changedArrowIds.append(arrow->getId());
                        int id = player->newArrowId();
                        arrow->setId(id);
                        arrowInfo->set_id(id);
                        arrowInfo->set_start_player_id(player->getPlayerId());
                        arrowInfo->set_start_zone(startCard->getZone()->getName().toStdString());
                        arrowInfo->set_start_card_id(startCard->getId());
                        const auto *arrowTargetPlayer = qobject_cast<const Server_AbstractPlayer *>(targetItem);
                        if (arrowTargetPlayer != nullptr) {
                            arrowInfo->set_target_player_id(arrowTargetPlayer->getPlayerId());
                        } else {
                            const auto *arrowTargetCard = qobject_cast<const Server_Card *>(targetItem);
                            arrowInfo->set_target_player_id(arrowTargetCard->getZone()->getPlayer()->getPlayerId());
                            arrowInfo->set_target_zone(arrowTargetCard->getZone()->getName().toStdString());
                            arrowInfo->set_target_card_id(arrowTargetCard->getId());
                        }
                        arrowInfo->mutable_arrow_color()->CopyFrom(arrow->getColor());
                        ges.enqueueGameEvent(_event, player->getPlayerId());
                    }
                }
                for (int id : changedArrowIds) {
                    player->updateArrowId(id);
                }
            }

            targetCard->resetState();
            card->setStashedCard(targetCard);
            break;
        }
    }

    return Response::RespOk;
}

/**
 * Creates and sends the events required to properly communicate the given token creation.
 * Primarily written to handle creating face-down tokens.
 */
void Server_AbstractPlayer::sendCreateTokenEvents(Server_CardZone *zone,
                                                  Server_Card *card,
                                                  int xCoord,
                                                  int yCoord,
                                                  GameEventStorage &ges)
{
    // Token is not face-down; things are easy
    if (!card->getFaceDown()) {
        ges.enqueueGameEvent(makeCreateTokenEvent(zone, card, xCoord, yCoord), playerId);
        return;
    }

    // Token is face-down. We have to send different info to each player
    auto eventOthers = makeCreateTokenEvent(zone, card, xCoord, yCoord, false);
    ges.enqueueGameEvent(eventOthers, playerId, GameEventStorageItem::SendToOthers);

    auto eventPrivate = makeCreateTokenEvent(zone, card, xCoord, yCoord, true);
    ges.enqueueGameEvent(eventPrivate, playerId, GameEventStorageItem::SendToPrivate, playerId);

    // Event_CreateToken didn't use to have face_down field; send attribute event afterward for backwards compatibility
    Event_SetCardAttr event;
    event.set_zone_name(zone->getName().toStdString());
    event.set_card_id(card->getId());
    event.set_attribute(AttrFaceDown);
    event.set_attr_value("1");
    ges.enqueueGameEvent(event, playerId);
}

Response::ResponseCode
Server_AbstractPlayer::cmdCreateArrow(const Command_CreateArrow &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }
    if (conceded) {
        return Response::RespContextError;
    }

    Server_AbstractPlayer *startPlayer = game->getPlayer(cmd.start_player_id());
    Server_AbstractPlayer *targetPlayer = game->getPlayer(cmd.target_player_id());
    if (!startPlayer || !targetPlayer) {
        return Response::RespNameNotFound;
    }
    QString startZoneName = nameFromStdString(cmd.start_zone());
    Server_CardZone *startZone = startPlayer->getZones().value(startZoneName);
    bool playerTarget = !cmd.has_target_zone();
    Server_CardZone *targetZone = nullptr;
    if (!playerTarget) {
        targetZone = targetPlayer->getZones().value(nameFromStdString(cmd.target_zone()));
    }
    if (!startZone || (!targetZone && !playerTarget)) {
        return Response::RespNameNotFound;
    }
    if (startZone->getType() != ServerInfo_Zone::PublicZone) {
        return Response::RespContextError;
    }
    Server_Card *startCard = startZone->getCard(cmd.start_card_id());
    if (!startCard) {
        return Response::RespNameNotFound;
    }
    Server_Card *targetCard = nullptr;
    if (!playerTarget) {
        if (targetZone->getType() != ServerInfo_Zone::PublicZone) {
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

    for (Server_Arrow *temp : arrows) {
        if ((temp->getStartCard() == startCard) && (temp->getTargetItem() == targetItem)) {
            return Response::RespContextError;
        }
    }

    int currentPhase = game->getActivePhase();
    int deletionPhase = cmd.has_delete_in_phase() ? cmd.delete_in_phase() : currentPhase;
    auto arrow = new Server_Arrow(newArrowId(), startCard, targetItem, cmd.arrow_color(), currentPhase, deletionPhase);
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
Server_AbstractPlayer::cmdDeleteArrow(const Command_DeleteArrow &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
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

Response::ResponseCode
Server_AbstractPlayer::cmdSetCardAttr(const Command_SetCardAttr &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }
    if (conceded) {
        return Response::RespContextError;
    }

    return setCardAttrHelper(ges, playerId, nameFromStdString(cmd.zone()), cmd.card_id(), cmd.attribute(),
                             nameFromStdString(cmd.attr_value()));
}

Response::ResponseCode Server_AbstractPlayer::cmdSetCardCounter(const Command_SetCardCounter &cmd,
                                                                ResponseContainer & /*rc*/,
                                                                GameEventStorage &ges)
{
    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }
    if (conceded) {
        return Response::RespContextError;
    }

    Server_CardZone *zone = zones.value(nameFromStdString(cmd.zone()));
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

    Event_SetCardCounter event;
    event.set_zone_name(zone->getName().toStdString());
    event.set_card_id(card->getId());
    card->setCounter(cmd.counter_id(), cmd.counter_value(), &event);
    ges.enqueueGameEvent(event, playerId);

    return Response::RespOk;
}

Response::ResponseCode Server_AbstractPlayer::cmdIncCardCounter(const Command_IncCardCounter &cmd,
                                                                ResponseContainer & /*rc*/,
                                                                GameEventStorage &ges)
{
    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }
    if (conceded) {
        return Response::RespContextError;
    }

    Server_CardZone *zone = zones.value(nameFromStdString(cmd.zone()));
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
Server_AbstractPlayer::cmdDumpZone(const Command_DumpZone &cmd, ResponseContainer &rc, GameEventStorage &ges)
{
    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }

    Server_AbstractPlayer *otherPlayer = game->getPlayer(cmd.player_id());
    if (!otherPlayer) {
        return Response::RespNameNotFound;
    }
    Server_CardZone *zone = otherPlayer->getZones().value(nameFromStdString(cmd.zone_name()));
    if (!zone) {
        return Response::RespNameNotFound;
    }
    if (!((zone->getType() == ServerInfo_Zone::PublicZone) || (this == otherPlayer))) {
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
        const auto &findId = cmd.is_reversed() ? cards.size() - numberCards + i : i;
        Server_Card *card = cards[findId];
        QString displayedName = card->getFaceDown() ? QString() : card->getName();
        ServerInfo_Card *cardInfo = zoneInfo->add_card_list();
        cardInfo->set_provider_id(card->getProviderId().toStdString());
        cardInfo->set_name(displayedName.toStdString());
        if (zone->getType() == ServerInfo_Zone::HiddenZone) {
            cardInfo->set_id(findId);
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
    if (zone->getType() == ServerInfo_Zone::HiddenZone) {
        zone->setCardsBeingLookedAt(numberCards);

        Event_DumpZone event;
        event.set_zone_owner_id(otherPlayer->getPlayerId());
        event.set_zone_name(zone->getName().toStdString());
        event.set_number_cards(numberCards);
        event.set_is_reversed(cmd.is_reversed());
        ges.enqueueGameEvent(event, playerId);
    }
    rc.setResponseExtension(re);
    return Response::RespOk;
}

Response::ResponseCode
Server_AbstractPlayer::cmdRevealCards(const Command_RevealCards &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }
    if (conceded) {
        return Response::RespContextError;
    }

    if (cmd.has_player_id()) {
        Server_AbstractPlayer *otherPlayer = game->getPlayer(cmd.player_id());
        if (!otherPlayer)
            return Response::RespNameNotFound;
    }
    Server_CardZone *zone = zones.value(nameFromStdString(cmd.zone_name()));
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
        cardInfo->set_provider_id(card->getProviderId().toStdString());
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

        if (isJudge()) {
            ges.setOverwriteOwnership(true);
        }

        ges.enqueueGameEvent(eventPrivate, playerId, GameEventStorageItem::SendToPrivate, cmd.player_id());
        ges.enqueueGameEvent(eventOthers, playerId, GameEventStorageItem::SendToOthers);
    } else {
        if (cmd.grant_write_access()) {
            const QList<int> &participantIds = game->getParticipants().keys();
            for (int anyParticipantId : participantIds) {
                zone->addWritePermission(anyParticipantId);
            }
        }

        ges.enqueueGameEvent(eventPrivate, playerId);
    }

    return Response::RespOk;
}

Response::ResponseCode Server_AbstractPlayer::cmdChangeZoneProperties(const Command_ChangeZoneProperties &cmd,
                                                                      ResponseContainer & /* rc */,
                                                                      GameEventStorage &ges)
{
    Server_CardZone *zone = zones.value(nameFromStdString(cmd.zone_name()));
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
    return Response::RespOk;
}

void Server_AbstractPlayer::getInfo(ServerInfo_Player *info,
                                    Server_AbstractParticipant *recipient,
                                    bool omniscient,
                                    bool withUserInfo)
{
    getProperties(*info->mutable_properties(), withUserInfo);
    if (recipient == this) {
        if (deck) {
            info->set_deck_list(deck->writeToString_Native().toStdString());
        }
    }

    for (Server_Arrow *arrow : arrows) {
        arrow->getInfo(info->add_arrow_list());
    }

    for (Server_CardZone *zone : zones) {
        zone->getInfo(info->add_zone_list(), recipient, omniscient);
    }
}

void Server_AbstractPlayer::getPlayerProperties(ServerInfo_PlayerProperties &result)
{
    result.set_conceded(conceded);
    result.set_sideboard_locked(sideboardLocked);
    result.set_ready_start(readyStart);
    if (deck) {
        result.set_deck_hash(deck->getDeckHash().toStdString());
    }
}
