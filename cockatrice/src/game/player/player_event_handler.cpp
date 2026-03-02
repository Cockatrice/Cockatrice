#include "player_event_handler.h"

#include "../../interface/widgets/tabs/tab_game.h"
#include "../board/arrow_item.h"
#include "../board/card_item.h"
#include "../board/card_list.h"
#include "../zones/view_zone.h"
#include "player.h"
#include "player_actions.h"

#include <libcockatrice/protocol/pb/command_set_card_attr.pb.h>
#include <libcockatrice/protocol/pb/context_move_card.pb.h>
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
#include <libcockatrice/protocol/pb/event_reveal_cards.pb.h>
#include <libcockatrice/protocol/pb/event_roll_die.pb.h>
#include <libcockatrice/protocol/pb/event_set_card_attr.pb.h>
#include <libcockatrice/protocol/pb/event_set_card_counter.pb.h>
#include <libcockatrice/protocol/pb/event_set_counter.pb.h>
#include <libcockatrice/protocol/pb/event_shuffle.pb.h>

PlayerEventHandler::PlayerEventHandler(Player *_player) : player(_player)
{
}

void PlayerEventHandler::eventGameSay(const Event_GameSay &event)
{
    emit logSay(player, QString::fromStdString(event.message()));
}

void PlayerEventHandler::eventShuffle(const Event_Shuffle &event)
{
    CardZoneLogic *zone = player->getZone(QString::fromStdString(event.zone_name()));
    if (!zone) {
        return;
    }
    auto &cardList = zone->getCards();
    int absStart = event.start();
    if (absStart < 0) { // negative indexes start from the end
        absStart += cardList.length();
    }

    // close all views that contain shuffled cards
    for (ZoneViewZone *view : zone->getViews()) {
        if (view != nullptr) {
            int length = view->getLogic()->getCards().length();
            // we want to close empty views as well
            if (length == 0 || length > absStart) { // note this assumes views always start at the top of the library
                view->close();
                break;
            }
        } else {
            qWarning() << zone->getName() << "of" << player->getPlayerInfo()->getName() << "holds empty zoneview!";
        }
    }

    // remove revealed card name on top of decks
    if (absStart == 0 && !cardList.isEmpty()) {
        cardList.first()->setCardRef({});
        emit zone->updateGraphics();
    }

    emit logShuffle(player, zone, event.start(), event.end());
}

void PlayerEventHandler::eventRollDie(const Event_RollDie &event)
{
    if (!event.values().empty()) {
        QList<uint> rolls(event.values().begin(), event.values().end());
        std::sort(rolls.begin(), rolls.end());
        emit logRollDie(player, static_cast<int>(event.sides()), rolls);
    } else if (event.value()) {
        // Backwards compatibility for old clients
        emit logRollDie(player, static_cast<int>(event.sides()), {event.value()});
    }
}

void PlayerEventHandler::eventCreateArrow(const Event_CreateArrow &event)
{
    ArrowItem *arrow = player->addArrow(event.arrow_info());
    if (!arrow) {
        return;
    }

    auto *startCard = static_cast<CardItem *>(arrow->getStartItem());
    auto *targetCard = qgraphicsitem_cast<CardItem *>(arrow->getTargetItem());
    if (targetCard) {
        emit logCreateArrow(player, startCard->getOwner(), startCard->getName(), targetCard->getOwner(),
                            targetCard->getName(), false);
    } else {
        emit logCreateArrow(player, startCard->getOwner(), startCard->getName(), arrow->getTargetItem()->getOwner(),
                            QString(), true);
    }
}

void PlayerEventHandler::eventDeleteArrow(const Event_DeleteArrow &event)
{
    player->delArrow(event.arrow_id());
}

void PlayerEventHandler::eventCreateToken(const Event_CreateToken &event)
{
    CardZoneLogic *zone = player->getZone(QString::fromStdString(event.zone_name()));
    if (!zone) {
        return;
    }

    CardRef cardRef = {QString::fromStdString(event.card_name()), QString::fromStdString(event.card_provider_id())};
    CardItem *card = new CardItem(player, nullptr, cardRef, event.card_id());
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

    emit logCreateToken(player, card->getName(), card->getPT(), card->getFaceDown());
    zone->addCard(card, true, event.x(), event.y());
}

void PlayerEventHandler::eventSetCardAttr(const Event_SetCardAttr &event,
                                          const GameEventContext &context,
                                          EventProcessingOptions options)
{
    CardZoneLogic *zone = player->getZone(QString::fromStdString(event.zone_name()));
    if (!zone) {
        return;
    }

    if (!event.has_card_id()) {
        const CardList &cards = zone->getCards();
        for (int i = 0; i < cards.size(); ++i) {
            player->getPlayerActions()->setCardAttrHelper(context, cards.at(i), event.attribute(),
                                                          QString::fromStdString(event.attr_value()), true, options);
        }
        if (event.attribute() == AttrTapped) {
            emit logSetTapped(player, nullptr, event.attr_value() == "1");
        }
    } else {
        CardItem *card = zone->getCard(event.card_id());
        if (!card) {
            qWarning() << "PlayerEventHandler::eventSetCardAttr: card id=" << event.card_id() << "not found";
            return;
        }
        player->getPlayerActions()->setCardAttrHelper(context, card, event.attribute(),
                                                      QString::fromStdString(event.attr_value()), false, options);
    }
}

void PlayerEventHandler::eventSetCardCounter(const Event_SetCardCounter &event)
{
    CardZoneLogic *zone = player->getZone(QString::fromStdString(event.zone_name()));
    if (!zone) {
        return;
    }

    CardItem *card = zone->getCard(event.card_id());
    if (!card) {
        return;
    }

    int oldValue = card->getCounters().value(event.counter_id(), 0);
    card->setCounter(event.counter_id(), event.counter_value());
    player->getPlayerMenu()->updateCardMenu(card);
    emit logSetCardCounter(player, card->getName(), event.counter_id(), event.counter_value(), oldValue);
}

void PlayerEventHandler::eventCreateCounter(const Event_CreateCounter &event)
{
    player->addCounter(event.counter_info());
}

void PlayerEventHandler::eventSetCounter(const Event_SetCounter &event)
{
    AbstractCounter *ctr = player->getCounters().value(event.counter_id(), 0);
    if (!ctr) {
        return;
    }
    int oldValue = ctr->getValue();
    ctr->setValue(event.value());
    emit logSetCounter(player, ctr->getName(), event.value(), oldValue);
}

void PlayerEventHandler::eventDelCounter(const Event_DelCounter &event)
{
    player->delCounter(event.counter_id());
}

void PlayerEventHandler::eventDumpZone(const Event_DumpZone &event)
{
    Player *zoneOwner = player->getGame()->getPlayerManager()->getPlayers().value(event.zone_owner_id(), 0);
    if (!zoneOwner) {
        return;
    }
    CardZoneLogic *zone = zoneOwner->getZones().value(QString::fromStdString(event.zone_name()), 0);
    if (!zone) {
        return;
    }
    emit logDumpZone(player, zone, event.number_cards(), event.is_reversed());
}

void PlayerEventHandler::eventMoveCard(const Event_MoveCard &event, const GameEventContext &context)
{
    Player *startPlayer = player->getGame()->getPlayerManager()->getPlayers().value(event.start_player_id());
    if (!startPlayer) {
        return;
    }
    QString startZoneString = QString::fromStdString(event.start_zone());
    CardZoneLogic *startZone = startPlayer->getZones().value(startZoneString, 0);
    Player *targetPlayer = player->getGame()->getPlayerManager()->getPlayers().value(event.target_player_id());
    if (!targetPlayer) {
        return;
    }
    CardZoneLogic *targetZone;
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
            emit targetZone->cardAdded(attachedCard);
        }

        if (startZone->getPlayer() != targetZone->getPlayer()) {
            card->setOwner(targetZone->getPlayer());
        }
    }

    // The log event has to be sent before the card is added to the target zone
    // because the addCard function can modify the card object.
    if (context.HasExtension(Context_UndoDraw::ext)) {
        emit logUndoDraw(player, card->getName());
    } else {
        emit logMoveCard(player, card, startZone, logPosition, targetZone, logX);
    }

    targetZone->addCard(card, true, x, y);

    // Look at all arrows from and to the card.
    // If the card was moved to another zone, delete the arrows, otherwise update them.
    QMapIterator<int, Player *> playerIterator(player->getGame()->getPlayerManager()->getPlayers());
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
    player->getPlayerMenu()->updateCardMenu(card);

    if (player->getPlayerActions()->isMovingCardsUntil() && startZoneString == "deck" &&
        targetZone->getName() == "stack") {
        player->getPlayerActions()->moveOneCardUntil(card);
    }
}

void PlayerEventHandler::eventFlipCard(const Event_FlipCard &event)
{
    CardZoneLogic *zone = player->getZone(QString::fromStdString(event.zone_name()));
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

    emit logFlipCard(player, card->getName(), event.face_down());
    card->setFaceDown(event.face_down());
    player->getPlayerMenu()->updateCardMenu(card);
}

void PlayerEventHandler::eventDestroyCard(const Event_DestroyCard &event)
{
    CardZoneLogic *zone = player->getZone(QString::fromStdString(event.zone_name()));
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

    emit logDestroyCard(player, card->getName());
    zone->takeCard(-1, event.card_id(), true);
    card->deleteLater();
}

void PlayerEventHandler::eventAttachCard(const Event_AttachCard &event)
{
    const QMap<int, Player *> &playerList = player->getGame()->getPlayerManager()->getPlayers();
    Player *targetPlayer = nullptr;
    CardZoneLogic *targetZone = nullptr;
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

    CardZoneLogic *startZone = player->getZone(QString::fromStdString(event.start_zone()));
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
        emit logAttachCard(player, startCard->getName(), targetPlayer, targetCard->getName());
    } else {
        emit logUnattachCard(player, startCard->getName());
    }
    player->getPlayerMenu()->updateCardMenu(startCard);
}

void PlayerEventHandler::eventDrawCards(const Event_DrawCards &event)
{
    CardZoneLogic *_deck = player->getDeckZone();
    CardZoneLogic *_hand = player->getHandZone();

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
    emit logDrawCards(player, event.number(), _deck->getCards().size() == 0);
}

void PlayerEventHandler::eventRevealCards(const Event_RevealCards &event, EventProcessingOptions options)
{
    Q_UNUSED(options);
    CardZoneLogic *zone = player->getZone(QString::fromStdString(event.zone_name()));
    if (!zone) {
        return;
    }
    Player *otherPlayer = nullptr;
    if (event.has_other_player_id()) {
        otherPlayer = player->getGame()->getPlayerManager()->getPlayers().value(event.other_player_id());
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
            emit logRevealCards(player, zone, card->id(), cardName, player, true, 1);
        }
    } else {
        bool showZoneView = true;
        QString cardName;
        auto cardId = event.card_id_size() == 0 ? -1 : event.card_id(0);
        if (cardList.size() == 1) {
            cardName = QString::fromStdString(cardList.first()->name());

            // Handle case of revealing top card of library in-place
            if (cardId == 0 && dynamic_cast<PileZoneLogic *>(zone)) {
                auto card = zone->getCards().first();
                QString providerId = QString::fromStdString(cardList.first()->provider_id());
                card->setCardRef({cardName, providerId});

                emit zone->updateGraphics();
                showZoneView = false;
            }
        }

        if (!options.testFlag(SKIP_REVEAL_WINDOW) && showZoneView && !cardList.isEmpty()) {
            player->getGameScene()->addRevealedZoneView(player, zone, cardList, event.grant_write_access());
        }

        emit logRevealCards(player, zone, cardId, cardName, otherPlayer, false,
                            event.has_number_of_cards() ? event.number_of_cards() : cardList.size(),
                            event.grant_write_access());
    }
}

void PlayerEventHandler::eventChangeZoneProperties(const Event_ChangeZoneProperties &event)
{
    CardZoneLogic *zone = player->getZone(QString::fromStdString(event.zone_name()));
    if (!zone) {
        return;
    }

    if (event.has_always_reveal_top_card()) {
        zone->setAlwaysRevealTopCard(event.always_reveal_top_card());
        emit logAlwaysRevealTopCard(player, zone, event.always_reveal_top_card());
    }
    if (event.has_always_look_at_top_card()) {
        zone->setAlwaysRevealTopCard(event.always_look_at_top_card());
        emit logAlwaysLookAtTopCard(player, zone, event.always_look_at_top_card());
    }
}

void PlayerEventHandler::processGameEvent(GameEvent::GameEventType type,
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