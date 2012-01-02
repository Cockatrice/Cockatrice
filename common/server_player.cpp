#include "server_player.h"
#include "server_card.h"
#include "server_counter.h"
#include "server_arrow.h"
#include "server_cardzone.h"
#include "server_game.h"
#include "server_protocolhandler.h"
#include "decklist.h"
#include "pb/response.pb.h"
#include "pb/command_move_card.pb.h"
#include "pb/serverinfo_user.pb.h"
#include "pb/event_attach_card.pb.h"
#include "pb/event_draw_cards.pb.h"
#include "pb/event_destroy_card.pb.h"
#include "pb/event_move_card.pb.h"
#include "pb/event_set_card_attr.pb.h"
#include "pb/context_move_card.pb.h"
#include <QDebug>

Server_Player::Server_Player(Server_Game *_game, int _playerId, const ServerInfo_User &_userInfo, bool _spectator, Server_ProtocolHandler *_handler)
	: game(_game), handler(_handler), userInfo(new ServerInfo_User(_userInfo)), deck(0), playerId(_playerId), spectator(_spectator), nextCardId(0), readyStart(false), conceded(false)
{
}

Server_Player::~Server_Player()
{
}

void Server_Player::prepareDestroy()
{
	QMutexLocker locker(&game->gameMutex);
	
	delete deck;
	
	playerMutex.lock();
	if (handler)
		handler->playerRemovedFromGame(game);
	playerMutex.unlock();
	
	delete userInfo;
	userInfo = 0;
	
	clearZones();
	
	deleteLater();
}

int Server_Player::newCardId()
{
	QMutexLocker locker(&game->gameMutex);
	
	return nextCardId++;
}

int Server_Player::newCounterId() const
{
	QMutexLocker locker(&game->gameMutex);
	
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
	QMutexLocker locker(&game->gameMutex);
	
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
	QMutexLocker locker(&game->gameMutex);
	
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
	addCounter(new Server_Counter(7, "storm", makeColor(255, 255, 255), 20, 0));

	initialCards = 7;

	// ------------------------------------------------------------------

	// Assign card ids and create deck from decklist
	InnerDecklistNode *listRoot = deck->getRoot();
	nextCardId = 0;
	for (int i = 0; i < listRoot->size(); ++i) {
		InnerDecklistNode *currentZone = dynamic_cast<InnerDecklistNode *>(listRoot->at(i));
		Server_CardZone *z;
		if (currentZone->getName() == "main")
			z = deckZone;
		else if (currentZone->getName() == "side")
			z = sbZone;
		else
			continue;
		
		for (int j = 0; j < currentZone->size(); ++j) {
			DecklistCardNode *currentCard = dynamic_cast<DecklistCardNode *>(currentZone->at(j));
			if (!currentCard)
				continue;
			for (int k = 0; k < currentCard->getNumber(); ++k)
				z->cards.append(new Server_Card(currentCard->getName(), nextCardId++, 0, 0, z));
		}
	}
	
	const QList<MoveCardToZone *> &sideboardPlan = deck->getCurrentSideboardPlan();
	for (int i = 0; i < sideboardPlan.size(); ++i) {
		MoveCardToZone *m = sideboardPlan[i];
		
		Server_CardZone *start, *target;
		if (m->getStartZone() == "main")
			start = deckZone;
		else if (m->getStartZone() == "side")
			start = sbZone;
		else
			continue;
		if (m->getTargetZone() == "main")
			target = deckZone;
		else if (m->getTargetZone() == "side")
			target = sbZone;
		else
			continue;
		
		for (int j = 0; j < start->cards.size(); ++j)
			if (start->cards[j]->getName() == m->getCardName()) {
				Server_Card *card = start->cards[j];
				start->cards.removeAt(j);
				target->cards.append(card);
				break;
			}
	}
	
	deckZone->shuffle();
}

void Server_Player::clearZones()
{
	QMutexLocker locker(&game->gameMutex);
	
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

ServerInfo_PlayerProperties Server_Player::getProperties()
{
	QMutexLocker locker(&game->gameMutex);
	
	ServerInfo_PlayerProperties result;
	result.set_player_id(playerId);
	result.mutable_user_info()->CopyFrom(*userInfo);
	result.set_spectator(spectator);
	result.set_conceded(conceded);
	result.set_ready_start(readyStart);
	if (deck)
		result.set_deck_hash(deck->getDeckHash().toStdString());
	
	return result;
}

void Server_Player::setDeck(DeckList *_deck)
{
	QMutexLocker locker(&game->gameMutex);
	
	delete deck;
	deck = _deck;
}

void Server_Player::addZone(Server_CardZone *zone)
{
	QMutexLocker locker(&game->gameMutex);
	
	zones.insert(zone->getName(), zone);
}

void Server_Player::addArrow(Server_Arrow *arrow)
{
	QMutexLocker locker(&game->gameMutex);
	
	arrows.insert(arrow->getId(), arrow);
}

bool Server_Player::deleteArrow(int arrowId)
{
	QMutexLocker locker(&game->gameMutex);
	
	Server_Arrow *arrow = arrows.value(arrowId, 0);
	if (!arrow)
		return false;
	arrows.remove(arrowId);
	delete arrow;
	return true;
}

void Server_Player::addCounter(Server_Counter *counter)
{
	QMutexLocker locker(&game->gameMutex);
	
	counters.insert(counter->getId(), counter);
}

bool Server_Player::deleteCounter(int counterId)
{
	QMutexLocker locker(&game->gameMutex);
	
	Server_Counter *counter = counters.value(counterId, 0);
	if (!counter)
		return false;
	counters.remove(counterId);
	delete counter;
	return true;
}

Response::ResponseCode Server_Player::drawCards(GameEventStorage &ges, int number)
{
	QMutexLocker locker(&game->gameMutex);
	
	Server_CardZone *deckZone = zones.value("deck");
	Server_CardZone *handZone = zones.value("hand");
	if (deckZone->cards.size() < number)
		number = deckZone->cards.size();
	
	Event_DrawCards eventOthers;
	eventOthers.set_number(number);
	Event_DrawCards eventPrivate(eventOthers);
	
	for (int i = 0; i < number; ++i) {
		Server_Card *card = deckZone->cards.takeFirst();
		handZone->cards.append(card);
		lastDrawList.append(card->getId());
		
		ServerInfo_Card *cardInfo = eventPrivate.add_cards();
		cardInfo->set_id(card->getId());
		cardInfo->set_name(card->getName().toStdString());
	}
	
	ges.enqueueGameEvent(eventPrivate, playerId, GameEventStorageItem::SendToPrivate, playerId);
	ges.enqueueGameEvent(eventOthers, playerId, GameEventStorageItem::SendToOthers);
	
	return Response::RespOk;
}

Response::ResponseCode Server_Player::undoDraw(GameEventStorage &ges)
{
	QMutexLocker locker(&game->gameMutex);
	
	if (lastDrawList.isEmpty())
		return Response::RespContextError;
	
	Response::ResponseCode retVal;
	CardToMove *cardToMove = new CardToMove;
	cardToMove->set_card_id(lastDrawList.takeLast());
	retVal = moveCard(ges, zones.value("hand"), QList<const CardToMove *>() << cardToMove, zones.value("deck"), 0, 0, false, true);
	delete cardToMove;
	return retVal;
}

Response::ResponseCode Server_Player::moveCard(GameEventStorage &ges, const QString &_startZone, const QList<const CardToMove *> &_cards, int targetPlayerId, const QString &_targetZone, int x, int y)
{
	QMutexLocker locker(&game->gameMutex);
	
	Server_CardZone *startzone = getZones().value(_startZone);
	Server_Player *targetPlayer = game->getPlayers().value(targetPlayerId);
	if (!targetPlayer)
		return Response::RespNameNotFound;
	Server_CardZone *targetzone = targetPlayer->getZones().value(_targetZone);
	if ((!startzone) || (!targetzone))
		return Response::RespNameNotFound;
	
	return moveCard(ges, startzone, _cards, targetzone, x, y);
}

class Server_Player::MoveCardCompareFunctor {
private:
	int x;
public:
	MoveCardCompareFunctor(int _x) : x(_x) { }
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

Response::ResponseCode Server_Player::moveCard(GameEventStorage &ges, Server_CardZone *startzone, const QList<const CardToMove *> &_cards, Server_CardZone *targetzone, int x, int y, bool fixFreeSpaces, bool undoingDraw)
{
	QMutexLocker locker(&game->gameMutex);
	
	// Disallow controller change to other zones than the table.
	if (((targetzone->getType() != ServerInfo_Zone::PublicZone) || !targetzone->hasCoords()) && (startzone->getPlayer() != targetzone->getPlayer()))
		return Response::RespContextError;
	
	if (!targetzone->hasCoords() && (x == -1))
		x = targetzone->cards.size();
	
	QList<QPair<Server_Card *, int> > cardsToMove;
	QMap<Server_Card *, const CardToMove *> cardProperties;
	for (int i = 0; i < _cards.size(); ++i) {
		int position;
		Server_Card *card = startzone->getCard(_cards[i]->card_id(), &position);
		if (!card)
			return Response::RespNameNotFound;
		if (!card->getAttachedCards().isEmpty() && !targetzone->isColumnEmpty(x, y))
			return Response::RespContextError;
		cardsToMove.append(QPair<Server_Card *, int>(card, position));
		cardProperties.insert(card, _cards[i]);
	}
	
	MoveCardCompareFunctor cmp(startzone == targetzone ? -1 : x);
	qSort(cardsToMove.begin(), cardsToMove.end(), cmp);
	
	bool secondHalf = false;
	int xIndex = -1;
	for (int cardIndex = 0; cardIndex < cardsToMove.size(); ++cardIndex) {
		Server_Card *card = cardsToMove[cardIndex].first;
		const CardToMove *thisCardProperties = cardProperties.value(card);
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
				card->setParentCard(0);
			
			// Make a copy of the list because the original one gets modified during the loop
			QList<Server_Card *> attachedCards = card->getAttachedCards();
			for (int i = 0; i < attachedCards.size(); ++i)
				attachedCards[i]->getZone()->getPlayer()->unattachCard(ges, attachedCards[i]);
		}
		
		if (startzone != targetzone) {
			// Delete all arrows from and to the card
			const QList<Server_Player *> &players = game->getPlayers().values();
			for (int i = 0; i < players.size(); ++i) {
				QList<int> arrowsToDelete;
				QMapIterator<int, Server_Arrow *> arrowIterator(players[i]->getArrows());
				while (arrowIterator.hasNext()) {
					Server_Arrow *arrow = arrowIterator.next().value();
					if ((arrow->getStartCard() == card) || (arrow->getTargetItem() == card))
						arrowsToDelete.append(arrow->getId());
				}
				for (int j = 0; j < arrowsToDelete.size(); ++j)
					players[i]->deleteArrow(arrowsToDelete[j]);
			}
		}
		
		if (card->getDestroyOnZoneChange() && (startzone->getName() != targetzone->getName())) {
			Event_DestroyCard event;
			event.set_zone_name(startzone->getName().toStdString());
			event.set_card_id(card->getId());
			ges.setGameEventContext(Context_MoveCard());
			ges.enqueueGameEvent(event, playerId);
			
			card->deleteLater();
		} else {
			if (!targetzone->hasCoords()) {
				y = 0;
				card->resetState();
			} else
				newX = targetzone->getFreeGridColumn(newX, y, card->getName());
		
			targetzone->insertCard(card, newX, y);
		
			bool targetBeingLookedAt = (targetzone->getType() != ServerInfo_Zone::HiddenZone) || (targetzone->getCardsBeingLookedAt() > newX) || (targetzone->getCardsBeingLookedAt() == -1);
			bool sourceBeingLookedAt = (startzone->getType() != ServerInfo_Zone::HiddenZone) || (startzone->getCardsBeingLookedAt() > position) || (startzone->getCardsBeingLookedAt() == -1);
		
			bool targetHiddenToPlayer = thisCardProperties->face_down() || !targetBeingLookedAt;
			bool targetHiddenToOthers = thisCardProperties->face_down() || (targetzone->getType() != ServerInfo_Zone::PublicZone);
			bool sourceHiddenToPlayer = card->getFaceDown() || !sourceBeingLookedAt;
			bool sourceHiddenToOthers = card->getFaceDown() || (startzone->getType() != ServerInfo_Zone::PublicZone);
		
			QString privateCardName, publicCardName;
			if (!(sourceHiddenToPlayer && targetHiddenToPlayer))
				privateCardName = card->getName();
			if (!(sourceHiddenToOthers && targetHiddenToOthers))
				publicCardName = card->getName();
		
			int oldCardId = card->getId();
			if (thisCardProperties->face_down() || (targetzone->getPlayer() != startzone->getPlayer()))
				card->setId(targetzone->getPlayer()->newCardId());
			card->setFaceDown(thisCardProperties->face_down());
		
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
			
			Event_MoveCard eventOthers;
			eventOthers.set_start_zone(startzone->getName().toStdString());
			eventOthers.set_target_player_id(targetzone->getPlayer()->getPlayerId());
			if (startzone != targetzone)
				eventOthers.set_target_zone(targetzone->getName().toStdString());
			eventOthers.set_x(newX);
			eventOthers.set_y(y);
			eventOthers.set_face_down(thisCardProperties->face_down());
			
			Event_MoveCard eventPrivate(eventOthers);
			eventPrivate.set_card_id(privateOldCardId);
			eventPrivate.set_card_name(privateCardName.toStdString());
			eventPrivate.set_position(privatePosition);
			eventPrivate.set_new_card_id(privateNewCardId);
			
			// Other players do not get to see the start and/or target position of the card if the respective
			// part of the zone is being looked at. The information is not needed anyway because in hidden zones,
			// all cards are equal.
			if (
				((startzone->getType() == ServerInfo_Zone::HiddenZone) && ((startzone->getCardsBeingLookedAt() > position) || (startzone->getCardsBeingLookedAt() == -1)))
				|| (startzone->getType() == ServerInfo_Zone::PublicZone)
			)
				position = -1;
			if ((targetzone->getType() == ServerInfo_Zone::HiddenZone) && ((targetzone->getCardsBeingLookedAt() > newX) || (targetzone->getCardsBeingLookedAt() == -1)))
				newX = -1;
			
			eventOthers.set_position(position);
			if ((startzone->getType() == ServerInfo_Zone::PublicZone) || (targetzone->getType() == ServerInfo_Zone::PublicZone)) {
				eventOthers.set_card_id(oldCardId);
				eventOthers.set_card_name(publicCardName.toStdString());
				eventOthers.set_new_card_id(card->getId());
			}
			
			ges.enqueueGameEvent(eventPrivate, playerId, GameEventStorageItem::SendToPrivate, playerId);
			ges.enqueueGameEvent(eventOthers, playerId, GameEventStorageItem::SendToOthers);
			
			if (thisCardProperties->tapped())
				setCardAttrHelper(ges, targetzone->getName(), card->getId(), AttrTapped, "1");
			QString ptString = QString::fromStdString(thisCardProperties->pt());
			if (!ptString.isEmpty() && !thisCardProperties->face_down())
				setCardAttrHelper(ges, targetzone->getName(), card->getId(), AttrPT, ptString);
		}
	}
	if (startzone->hasCoords() && fixFreeSpaces)
		startzone->fixFreeSpaces(ges);
	
	return Response::RespOk;
}

void Server_Player::unattachCard(GameEventStorage &ges, Server_Card *card)
{
	QMutexLocker locker(&game->gameMutex);
	
	Server_CardZone *zone = card->getZone();
	
	card->setParentCard(0);
	
	Event_AttachCard event;
	event.set_start_zone(zone->getName().toStdString());
	event.set_card_id(card->getId());
	ges.enqueueGameEvent(event, playerId);
	
	CardToMove *cardToMove = new CardToMove;
	cardToMove->set_card_id(card->getId());
	moveCard(ges, zone, QList<const CardToMove *>() << cardToMove, zone, -1, card->getY(), card->getFaceDown());
	delete cardToMove;
}

Response::ResponseCode Server_Player::setCardAttrHelper(GameEventStorage &ges, const QString &zoneName, int cardId, CardAttribute attribute, const QString &attrValue)
{
	QMutexLocker locker(&game->gameMutex);
	
	Server_CardZone *zone = getZones().value(zoneName);
	if (!zone)
		return Response::RespNameNotFound;
	if (!zone->hasCoords())
		return Response::RespContextError;

	QString result;
	if (cardId == -1) {
		QListIterator<Server_Card *> CardIterator(zone->cards);
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
	ges.enqueueGameEvent(event, playerId);
	
	return Response::RespOk;
}

void Server_Player::sendGameEvent(GameEventContainer *cont)
{
	QMutexLocker locker(&playerMutex);
	
	if (handler)
		handler->sendProtocolItem(*cont);
}
