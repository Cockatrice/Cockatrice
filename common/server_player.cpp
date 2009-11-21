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

Server_Player::Server_Player(Server_Game *_game, int _playerId, const QString &_playerName, bool _spectator, Server_ProtocolHandler *_handler)
	: game(_game), handler(_handler), deck(0), playerId(_playerId), playerName(_playerName), spectator(_spectator), nextCardId(0), PlayerStatus(StatusNormal)
{
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
	// Delete existing zones and counters
	clearZones();

	// This may need to be customized according to the game rules.
	// ------------------------------------------------------------------

	// Create zones
	Server_CardZone *deckZone = new Server_CardZone(this, "deck", false, Server_CardZone::HiddenZone);
	addZone(deckZone);
	Server_CardZone *sbZone = new Server_CardZone(this, "sb", false, Server_CardZone::HiddenZone);
	addZone(sbZone);
	addZone(new Server_CardZone(this, "table", true, Server_CardZone::PublicZone));
	addZone(new Server_CardZone(this, "hand", false, Server_CardZone::PrivateZone));
	addZone(new Server_CardZone(this, "grave", false, Server_CardZone::PublicZone));
	addZone(new Server_CardZone(this, "rfg", false, Server_CardZone::PublicZone));

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
	deckZone->shuffle();

	PlayerStatus = StatusPlaying;
	game->sendGameEvent(new Event_SetupZones(-1, playerId, deckZone->cards.size(), sbZone->cards.size()));
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

void Server_Player::setDeck(DeckList *_deck)
{
	delete deck;
	deck = _deck;
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

void Server_Player::sendProtocolItem(ProtocolItem *item, bool deleteItem)
{
	if (handler)
		handler->sendProtocolItem(item, deleteItem);
}
