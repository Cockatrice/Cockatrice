#include "server_player.h"
#include "server_card.h"
#include "server_counter.h"
#include "server_arrow.h"
#include "server_cardzone.h"
#include "server_game.h"
#include "server_protocolhandler.h"
#include "protocol.h"
#include "protocol_items.h"
#include "decklist.h"
#include <QDebug>

Server_Player::Server_Player(Server_Game *_game, int _playerId, ServerInfo_User *_userInfo, bool _spectator, Server_ProtocolHandler *_handler)
	: game(_game), handler(_handler), userInfo(new ServerInfo_User(_userInfo)), deck(0), playerId(_playerId), spectator(_spectator), nextCardId(0), readyStart(false), conceded(false), deckId(-2)
{
}

Server_Player::~Server_Player()
{
	delete deck;
	
	if (handler)
		handler->playerRemovedFromGame(game);
	delete userInfo;
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
	Server_CardZone *deckZone = new Server_CardZone(this, "deck", false, HiddenZone);
	addZone(deckZone);
	Server_CardZone *sbZone = new Server_CardZone(this, "sb", false, HiddenZone);
	addZone(sbZone);
	addZone(new Server_CardZone(this, "table", true, PublicZone));
	addZone(new Server_CardZone(this, "hand", false, PrivateZone));
	addZone(new Server_CardZone(this, "stack", false, PublicZone));
	addZone(new Server_CardZone(this, "grave", false, PublicZone));
	addZone(new Server_CardZone(this, "rfg", false, PublicZone));

	addCounter(new Server_Counter(0, "life", Color(255, 255, 255), 25, 20));
	addCounter(new Server_Counter(1, "w", Color(255, 255, 150), 20, 0));
	addCounter(new Server_Counter(2, "u", Color(150, 150, 255), 20, 0));
	addCounter(new Server_Counter(3, "b", Color(150, 150, 150), 20, 0));
	addCounter(new Server_Counter(4, "r", Color(250, 150, 150), 20, 0));
	addCounter(new Server_Counter(5, "g", Color(150, 255, 150), 20, 0));
	addCounter(new Server_Counter(6, "x", Color(255, 255, 255), 20, 0));
	addCounter(new Server_Counter(7, "storm", Color(255, 255, 255), 20, 0));

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
				z->cards.append(new Server_Card(currentCard->getName(), nextCardId++, 0, 0));
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
}

ServerInfo_PlayerProperties *Server_Player::getProperties()
{
	return new ServerInfo_PlayerProperties(playerId, new ServerInfo_User(userInfo), spectator, conceded, readyStart, deckId);
}

void Server_Player::setDeck(DeckList *_deck, int _deckId)
{
	delete deck;
	deck = _deck;
	deckId = _deckId;
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

bool Server_Player::deleteCounter(int counterId)
{
	Server_Counter *counter = counters.value(counterId, 0);
	if (!counter)
		return false;
	counters.remove(counterId);
	delete counter;
	return true;
}

ResponseCode Server_Player::moveCard(CommandContainer *cont, const QString &_startZone, const QList<int> &_cardIds, int targetPlayerId, const QString &_targetZone, int x, int y, bool faceDown, bool tapped)
{
	Server_CardZone *startzone = getZones().value(_startZone);
	Server_Player *targetPlayer = game->getPlayers().value(targetPlayerId);
	if (!targetPlayer)
		return RespNameNotFound;
	Server_CardZone *targetzone = targetPlayer->getZones().value(_targetZone);
	if ((!startzone) || (!targetzone))
		return RespNameNotFound;
	
	return moveCard(cont, startzone, _cardIds, targetzone, x, y, faceDown, tapped);
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

ResponseCode Server_Player::moveCard(CommandContainer *cont, Server_CardZone *startzone, const QList<int> &_cardIds, Server_CardZone *targetzone, int x, int y, bool faceDown, bool tapped, bool fixFreeSpaces)
{
	// Disallow controller change to other zones than the table.
	if (((targetzone->getType() != PublicZone) || !targetzone->hasCoords()) && (startzone->getPlayer() != targetzone->getPlayer()))
		return RespContextError;
	
	if (!targetzone->hasCoords() && (x == -1))
		x = targetzone->cards.size();
	
	QList<QPair<Server_Card *, int> > cardsToMove;
	for (int i = 0; i < _cardIds.size(); ++i) {
		int position;
		Server_Card *card = startzone->getCard(_cardIds[i], false, &position);
		if (!card)
			return RespNameNotFound;
		if (!card->getAttachedCards().isEmpty() && !targetzone->isColumnEmpty(x, y))
			return RespContextError;
		cardsToMove.append(QPair<Server_Card *, int>(card, position));
	}
	
	MoveCardCompareFunctor cmp(startzone == targetzone ? -1 : x);
	qSort(cardsToMove.begin(), cardsToMove.end(), cmp);
	
	bool secondHalf = false;
	int xIndex = -1;
	for (int cardIndex = 0; cardIndex < cardsToMove.size(); ++cardIndex) {
		Server_Card *card = cardsToMove[cardIndex].first;
		int originalPosition = cardsToMove[cardIndex].second;
		int position = startzone->removeCard(card);
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
				attachedCards[i]->getZone()->getPlayer()->unattachCard(cont, attachedCards[i]);
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
			cont->enqueueGameEventPrivate(new Event_DestroyCard(getPlayerId(), startzone->getName(), card->getId()), game->getGameId());
			cont->enqueueGameEventPublic(new Event_DestroyCard(getPlayerId(), startzone->getName(), card->getId()), game->getGameId());
			card->deleteLater();
		} else {
			if (!targetzone->hasCoords()) {
				y = 0;
				card->resetState();
			} else
				newX = targetzone->getFreeGridColumn(newX, y, card->getName());
		
			targetzone->insertCard(card, newX, y);
		
			bool targetBeingLookedAt = (targetzone->getType() != HiddenZone) || (targetzone->getCardsBeingLookedAt() > newX) || (targetzone->getCardsBeingLookedAt() == -1);
			bool sourceBeingLookedAt = (startzone->getType() != HiddenZone) || (startzone->getCardsBeingLookedAt() > position) || (startzone->getCardsBeingLookedAt() == -1);
		
			bool targetHiddenToPlayer = faceDown || !targetBeingLookedAt;
			bool targetHiddenToOthers = faceDown || (targetzone->getType() != PublicZone);
			bool sourceHiddenToPlayer = card->getFaceDown() || !sourceBeingLookedAt;
			bool sourceHiddenToOthers = card->getFaceDown() || (startzone->getType() != PublicZone);
		
			QString privateCardName, publicCardName;
			if (!(sourceHiddenToPlayer && targetHiddenToPlayer))
				privateCardName = card->getName();
			if (!(sourceHiddenToOthers && targetHiddenToOthers))
				publicCardName = card->getName();
		
			int oldCardId = card->getId();
			if (faceDown || (targetzone->getPlayer() != startzone->getPlayer()))
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
			if (startzone->getType() == HiddenZone)
				privatePosition = position;
			cont->enqueueGameEventPrivate(new Event_MoveCard(getPlayerId(), privateOldCardId, privateCardName, startzone->getName(), privatePosition, targetzone->getPlayer()->getPlayerId(), targetzone->getName(), newX, y, privateNewCardId, faceDown), game->getGameId());
			cont->enqueueGameEventOmniscient(new Event_MoveCard(getPlayerId(), privateOldCardId, privateCardName, startzone->getName(), privatePosition, targetzone->getPlayer()->getPlayerId(), targetzone->getName(), newX, y, privateNewCardId, faceDown), game->getGameId());
			
			// Other players do not get to see the start and/or target position of the card if the respective
			// part of the zone is being looked at. The information is not needed anyway because in hidden zones,
			// all cards are equal.
			if (
				((startzone->getType() == HiddenZone) && ((startzone->getCardsBeingLookedAt() > position) || (startzone->getCardsBeingLookedAt() == -1)))
				|| (startzone->getType() == PublicZone)
			)
				position = -1;
			if ((targetzone->getType() == HiddenZone) && ((targetzone->getCardsBeingLookedAt() > newX) || (targetzone->getCardsBeingLookedAt() == -1)))
				newX = -1;
		
			if ((startzone->getType() == PublicZone) || (targetzone->getType() == PublicZone))
				cont->enqueueGameEventPublic(new Event_MoveCard(getPlayerId(), oldCardId, publicCardName, startzone->getName(), position, targetzone->getPlayer()->getPlayerId(), targetzone->getName(), newX, y, card->getId(), faceDown), game->getGameId());
			else
				cont->enqueueGameEventPublic(new Event_MoveCard(getPlayerId(), -1, QString(), startzone->getName(), position, targetzone->getPlayer()->getPlayerId(), targetzone->getName(), newX, y, -1, false), game->getGameId());
			
			if (tapped)
				setCardAttrHelper(cont, targetzone->getName(), card->getId(), "tapped", "1");
		}
	}
	if (startzone->hasCoords() && fixFreeSpaces)
		startzone->fixFreeSpaces(cont);
	
	return RespOk;
}

void Server_Player::unattachCard(CommandContainer *cont, Server_Card *card)
{
	Server_CardZone *zone = card->getZone();
	
	card->setParentCard(0);
	cont->enqueueGameEventPrivate(new Event_AttachCard(getPlayerId(), zone->getName(), card->getId(), -1, QString(), -1), game->getGameId());
	cont->enqueueGameEventPublic(new Event_AttachCard(getPlayerId(), zone->getName(), card->getId(), -1, QString(), -1), game->getGameId());
	
	moveCard(cont, zone, QList<int>() << card->getId(), zone, -1, card->getY(), card->getFaceDown(), false);
}

ResponseCode Server_Player::setCardAttrHelper(CommandContainer *cont, const QString &zoneName, int cardId, const QString &attrName, const QString &attrValue)
{
	Server_CardZone *zone = getZones().value(zoneName);
	if (!zone)
		return RespNameNotFound;
	if (!zone->hasCoords())
		return RespContextError;

	if (cardId == -1) {
		QListIterator<Server_Card *> CardIterator(zone->cards);
		while (CardIterator.hasNext())
			if (!CardIterator.next()->setAttribute(attrName, attrValue, true))
				return RespInvalidCommand;
	} else {
		Server_Card *card = zone->getCard(cardId, false);
		if (!card)
			return RespNameNotFound;
		if (!card->setAttribute(attrName, attrValue, false))
			return RespInvalidCommand;
	}
	cont->enqueueGameEventPrivate(new Event_SetCardAttr(getPlayerId(), zone->getName(), cardId, attrName, attrValue), game->getGameId());
	cont->enqueueGameEventPublic(new Event_SetCardAttr(getPlayerId(), zone->getName(), cardId, attrName, attrValue), game->getGameId());
	cont->enqueueGameEventOmniscient(new Event_SetCardAttr(getPlayerId(), zone->getName(), cardId, attrName, attrValue), game->getGameId());
	return RespOk;
}

void Server_Player::sendProtocolItem(ProtocolItem *item, bool deleteItem)
{
	if (handler)
		handler->sendProtocolItem(item, deleteItem);
}
