#include "server_player.h"
#include "server_card.h"
#include "server_counter.h"
#include "server_arrow.h"
#include "server_cardzone.h"
#include "server_game.h"

Server_Player::Server_Player(Server_Game *_game, int _playerId, const QString &_playerName, bool _spectator)
	: game(_game), socket(0), playerId(_playerId), playerName(_playerName), spectator(_spectator), nextCardId(0), PlayerStatus(StatusNormal)
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
	Server_CardZone *deck = new Server_CardZone(this, "deck", false, Server_CardZone::HiddenZone);
	addZone(deck);
	Server_CardZone *sb = new Server_CardZone(this, "sb", false, Server_CardZone::HiddenZone);
	addZone(sb);
	addZone(new Server_CardZone(this, "table", true, Server_CardZone::PublicZone));
	addZone(new Server_CardZone(this, "hand", false, Server_CardZone::PrivateZone));
	addZone(new Server_CardZone(this, "grave", false, Server_CardZone::PublicZone));
	addZone(new Server_CardZone(this, "rfg", false, Server_CardZone::PublicZone));

	// ------------------------------------------------------------------

	// Assign card ids and create deck from decklist
	QListIterator<QString> DeckIterator(DeckList);
	int i = 0;
	while (DeckIterator.hasNext())
		deck->cards.append(new Server_Card(DeckIterator.next(), i++, 0, 0));
	deck->shuffle();

	QListIterator<QString> SBIterator(SideboardList);
	while (SBIterator.hasNext())
		sb->cards.append(new Server_Card(SBIterator.next(), i++, 0, 0));

	nextCardId = i;
	
	PlayerStatus = StatusPlaying;
	game->broadcastEvent(QString("setup_zones|%1|%2").arg(deck->cards.size())
						   .arg(sb->cards.size()), this);
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

void Server_Player::privateEvent(const QString &line)
{
/*	if (!socket)
		return;
	socket->msg(QString("private|%1|%2|%3").arg(playerId).arg(playerName).arg(line));
*/}

void Server_Player::publicEvent(const QString &line, Server_Player *player)
{
/*	if (!socket)
		return;
	if (player)
		socket->msg(QString("public|%1|%2|%3").arg(player->getPlayerId()).arg(player->getPlayerName()).arg(line));
	else
		socket->msg(QString("public|||%1").arg(line));
*/}
