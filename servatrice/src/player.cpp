#include "player.h"
#include "card.h"
#include "counter.h"
#include "arrow.h"
#include "playerzone.h"
#include "serversocket.h"
#include "servergame.h"

Player::Player(ServerGame *_game, int _playerId, const QString &_playerName, bool _spectator)
	: game(_game), socket(0), playerId(_playerId), playerName(_playerName), spectator(_spectator), nextCardId(0), PlayerStatus(StatusNormal)
{
}

int Player::newCardId()
{
	return nextCardId++;
}

int Player::newCounterId() const
{
	int id = 0;
	QMapIterator<int, Counter *> i(counters);
	while (i.hasNext()) {
		Counter *c = i.next().value();
		if (c->getId() > id)
			id = c->getId();
	}
	return id + 1;
}

int Player::newArrowId() const
{
	int id = 0;
	QMapIterator<int, Arrow *> i(arrows);
	while (i.hasNext()) {
		Arrow *a = i.next().value();
		if (a->getId() > id)
			id = a->getId();
	}
	return id + 1;
}

void Player::setupZones()
{
	// Delete existing zones and counters
	clearZones();

	// This may need to be customized according to the game rules.
	// ------------------------------------------------------------------

	// Create zones
	PlayerZone *deck = new PlayerZone(this, "deck", false, PlayerZone::HiddenZone);
	addZone(deck);
	PlayerZone *sb = new PlayerZone(this, "sb", false, PlayerZone::HiddenZone);
	addZone(sb);
	addZone(new PlayerZone(this, "table", true, PlayerZone::PublicZone));
	addZone(new PlayerZone(this, "hand", false, PlayerZone::PrivateZone));
	addZone(new PlayerZone(this, "grave", false, PlayerZone::PublicZone));
	addZone(new PlayerZone(this, "rfg", false, PlayerZone::PublicZone));

	// ------------------------------------------------------------------

	// Assign card ids and create deck from decklist
	QListIterator<QString> DeckIterator(DeckList);
	int i = 0;
	while (DeckIterator.hasNext())
		deck->cards.append(new Card(DeckIterator.next(), i++, 0, 0));
	deck->shuffle();

	QListIterator<QString> SBIterator(SideboardList);
	while (SBIterator.hasNext())
		sb->cards.append(new Card(SBIterator.next(), i++, 0, 0));

	nextCardId = i;
	
	PlayerStatus = StatusPlaying;
	game->broadcastEvent(QString("setup_zones|%1|%2").arg(deck->cards.size())
						   .arg(sb->cards.size()), this);
}

void Player::clearZones()
{
	QMapIterator<QString, PlayerZone *> zoneIterator(zones);
	while (zoneIterator.hasNext())
		delete zoneIterator.next().value();
	zones.clear();

	QMapIterator<int, Counter *> counterIterator(counters);
	while (counterIterator.hasNext())
		delete counterIterator.next().value();
	counters.clear();
	
	QMapIterator<int, Arrow *> arrowIterator(arrows);
	while (arrowIterator.hasNext())
		delete arrowIterator.next().value();
	arrows.clear();
}

void Player::addZone(PlayerZone *zone)
{
	zones.insert(zone->getName(), zone);
}

void Player::addArrow(Arrow *arrow)
{
	arrows.insert(arrow->getId(), arrow);
}

bool Player::deleteArrow(int arrowId)
{
	Arrow *arrow = arrows.value(arrowId, 0);
	if (!arrow)
		return false;
	arrows.remove(arrowId);
	delete arrow;
	return true;
}

void Player::addCounter(Counter *counter)
{
	counters.insert(counter->getId(), counter);
}

bool Player::deleteCounter(int counterId)
{
	Counter *counter = counters.value(counterId, 0);
	if (!counter)
		return false;
	counters.remove(counterId);
	delete counter;
	return true;
}

void Player::privateEvent(const QString &line)
{
	if (!socket)
		return;
	socket->msg(QString("private|%1|%2|%3").arg(playerId).arg(playerName).arg(line));
}

void Player::publicEvent(const QString &line, Player *player)
{
	if (!socket)
		return;
	if (player)
		socket->msg(QString("public|%1|%2|%3").arg(player->getPlayerId()).arg(player->getPlayerName()).arg(line));
	else
		socket->msg(QString("public|||%1").arg(line));
}
