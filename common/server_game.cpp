/***************************************************************************
 *   Copyright (C) 2008 by Max-Wilhelm Bruker   *
 *   brukie@laptop   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "server.h"
#include "server_game.h"
#include "server_protocolhandler.h"
#include "server_arrow.h"
#include "server_card.h"
#include "server_cardzone.h"
#include "server_counter.h"
#include <QSqlQuery>
#include <QTimer>

Server_Game::Server_Game(Server_ProtocolHandler *_creator, int _gameId, const QString &_description, const QString &_password, int _maxPlayers, bool _spectatorsAllowed, QObject *parent)
	: QObject(parent), gameStarted(false), gameId(_gameId), description(_description), password(_password), maxPlayers(_maxPlayers), activePlayer(-1), activePhase(-1), spectatorsAllowed(_spectatorsAllowed), inactivityCounter(0)
{
	creator = addPlayer(_creator, false, false);

	pingClock = new QTimer(this);
	connect(pingClock, SIGNAL(timeout()), this, SLOT(pingClockTimeout()));
	pingClock->start(1000);
}

Server_Game::~Server_Game()
{
	sendGameEvent(new Event_GameClosed);
	
	QMapIterator<int, Server_Player *> playerIterator(players);
	while (playerIterator.hasNext())
		delete playerIterator.next().value();
	players.clear();
	
	emit gameClosing();
	qDebug("Server_Game destructor");
}

void Server_Game::pingClockTimeout()
{
	QDateTime now = QDateTime::currentDateTime();
	QList<ServerInfo_PlayerPing *> pingList;
	QMapIterator<int, Server_Player *> playerIterator(players);
	bool allPlayersInactive = true;
	while (playerIterator.hasNext()) {
		Server_Player *player = playerIterator.next().value();
		int pingTime;
		if (player->getProtocolHandler()) {
			pingTime = player->getProtocolHandler()->getLastCommandTime().secsTo(now);
			allPlayersInactive = false;
		} else
			pingTime = -1;
		pingList.append(new ServerInfo_PlayerPing(player->getPlayerId(), pingTime));
	}
	sendGameEvent(new Event_Ping(-1, pingList));
	
	const int maxTime = static_cast<Server *>(parent())->getMaxGameInactivityTime();
	if (allPlayersInactive) {
		if ((++inactivityCounter >= maxTime) && (maxTime > 0))
			deleteLater();
	} else
		inactivityCounter = 0;
}

int Server_Game::getPlayerCount() const
{
	QMapIterator<int, Server_Player *> playerIterator(players);
	int result = 0;
	while (playerIterator.hasNext())
		if (!playerIterator.next().value()->getSpectator())
			++result;
	return result;
}

int Server_Game::getSpectatorCount() const
{
	QMapIterator<int, Server_Player *> playerIterator(players);
	int result = 0;
	while (playerIterator.hasNext())
		if (playerIterator.next().value()->getSpectator())
			++result;
	return result;
}

void Server_Game::startGameIfReady()
{
	if (getPlayerCount() < maxPlayers)
		return;
	QMapIterator<int, Server_Player *> playerIterator(players);
	while (playerIterator.hasNext()) {
		Server_Player *p = playerIterator.next().value();
		if (!p->getReadyStart() && !p->getSpectator())
			return;
	}
	playerIterator.toFront();
	while (playerIterator.hasNext()) {
		Server_Player *p = playerIterator.next().value();
		if (!p->getSpectator())
			p->setupZones();
	}

	gameStarted = true;
	playerIterator.toFront();
	while (playerIterator.hasNext()) {
		Server_Player *player = playerIterator.next().value();
		player->setConceded(false);
		player->setReadyStart(false);
		player->sendProtocolItem(new Event_GameStateChanged(gameId, gameStarted, 0, 0, getGameState(player)));
	}
	
/*	QSqlQuery query;
	query.prepare("insert into games (id, descr, password, time_started) values(:id, :descr, :password, now())");
	query.bindValue(":id", gameId);
	query.bindValue(":descr", description);
	query.bindValue(":password", !password.isEmpty());
	query.exec();
	
	QMapIterator<int, Server_Player *> playerIterator2(players);
	while (playerIterator2.hasNext()) {
		Server_Player *player = playerIterator2.next().value();
		query.prepare("insert into games_players (id_game, player) values(:id, :player)");
		query.bindValue(":id", gameId);
		query.bindValue(":player", player->getPlayerName());
		query.exec();
	}
*/	
	setActivePlayer(0);
}

void Server_Game::stopGameIfFinished()
{
	QMapIterator<int, Server_Player *> playerIterator(players);
	int playing = 0;
	while (playerIterator.hasNext()) {
		Server_Player *p = playerIterator.next().value();
		if (!p->getConceded() && !p->getSpectator())
			++playing;
	}
	if (playing > 1)
		return;

	gameStarted = false;
	playerIterator.toFront();
	while (playerIterator.hasNext()) {
		Server_Player *player = playerIterator.next().value();
		player->sendProtocolItem(new Event_GameStateChanged(gameId, gameStarted, -1, -1, getGameState(player)));
	}
}

ResponseCode Server_Game::checkJoin(const QString &_password, bool spectator)
{
	if (_password != password)
		return RespWrongPassword;
	if (spectator) {
		if (!spectatorsAllowed)
			return RespSpectatorsNotAllowed;
	} else if (gameStarted || (getPlayerCount() >= getMaxPlayers()))
		return RespGameFull;
	
	return RespOk;
}

Server_Player *Server_Game::addPlayer(Server_ProtocolHandler *handler, bool spectator, bool broadcastUpdate)
{
	const QList<int> &keyList = players.keys();
	int playerId = keyList.isEmpty() ? 0 : (keyList.last() + 1);
	
	Server_Player *newPlayer = new Server_Player(this, playerId, handler->getPlayerName(), spectator, handler);
	sendGameEvent(new Event_Join(-1, new ServerInfo_Player(playerId, handler->getPlayerName(), spectator)));
	players.insert(playerId, newPlayer);

	if (broadcastUpdate)
		qobject_cast<Server *>(parent())->broadcastGameListUpdate(this);
	
	return newPlayer;
}

void Server_Game::removePlayer(Server_Player *player)
{
	players.remove(player->getPlayerId());
	sendGameEvent(new Event_Leave(-1, player->getPlayerId()));
	delete player;
	
	if (!getPlayerCount())
		deleteLater();
	else
		stopGameIfFinished();
	qobject_cast<Server *>(parent())->broadcastGameListUpdate(this);
}

void Server_Game::setActivePlayer(int _activePlayer)
{
	activePlayer = _activePlayer;
	sendGameEvent(new Event_SetActivePlayer(-1, -1, activePlayer));
	setActivePhase(0);
}

void Server_Game::setActivePhase(int _activePhase)
{
	QMapIterator<int, Server_Player *> playerIterator(players);
	while (playerIterator.hasNext()) {
		Server_Player *player = playerIterator.next().value();
		QList<Server_Arrow *> toDelete = player->getArrows().values();
		for (int i = 0; i < toDelete.size(); ++i) {
			Server_Arrow *a = toDelete[i];
			sendGameEvent(new Event_DeleteArrow(-1, player->getPlayerId(), a->getId()));
			player->deleteArrow(a->getId());
		}
	}
	
	activePhase = _activePhase;
	sendGameEvent(new Event_SetActivePhase(-1, -1, activePhase));
}

QList<ServerInfo_Player *> Server_Game::getGameState(Server_Player *playerWhosAsking) const
{
	QList<ServerInfo_Player *> result;
	QMapIterator<int, Server_Player *> playerIterator(players);
	while (playerIterator.hasNext()) {
		Server_Player *player = playerIterator.next().value();

		QList<ServerInfo_Arrow *> arrowList;
		QMapIterator<int, Server_Arrow *> arrowIterator(player->getArrows());
		while (arrowIterator.hasNext()) {
			Server_Arrow *arrow = arrowIterator.next().value();
			arrowList.append(new ServerInfo_Arrow(
				arrow->getId(),
				arrow->getStartCard()->getZone()->getPlayer()->getPlayerId(),
				arrow->getStartCard()->getZone()->getName(),
				arrow->getStartCard()->getId(),
				arrow->getTargetCard()->getZone()->getPlayer()->getPlayerId(),
				arrow->getTargetCard()->getZone()->getName(),
				arrow->getTargetCard()->getId(),
				arrow->getColor()
			));
		}

		QList<ServerInfo_Counter *> counterList;
		QMapIterator<int, Server_Counter *> counterIterator(player->getCounters());
		while (counterIterator.hasNext()) {
			Server_Counter *counter = counterIterator.next().value();
			counterList.append(new ServerInfo_Counter(counter->getId(), counter->getName(), counter->getColor(), counter->getRadius(), counter->getCount()));
		}

		QList<ServerInfo_Zone *> zoneList;
		QMapIterator<QString, Server_CardZone *> zoneIterator(player->getZones());
		while (zoneIterator.hasNext()) {
			Server_CardZone *zone = zoneIterator.next().value();
			QList<ServerInfo_Card *> cardList;
			if (
				((playerWhosAsking == player) && (zone->getType() != HiddenZone))
				|| ((playerWhosAsking != player) && (zone->getType() == PublicZone))
			) {
				QListIterator<Server_Card *> cardIterator(zone->cards);
				while (cardIterator.hasNext()) {
					Server_Card *card = cardIterator.next();
					cardList.append(new ServerInfo_Card(card->getId(), card->getName(), card->getX(), card->getY(), card->getCounters(), card->getTapped(), card->getAttacking(), card->getAnnotation()));
				}
			}
			zoneList.append(new ServerInfo_Zone(zone->getName(), zone->getType(), zone->hasCoords(), zone->cards.size(), cardList));
		}

		result.append(new ServerInfo_Player(player->getPlayerId(), player->getPlayerName(), player->getSpectator(), player->getConceded(), player == playerWhosAsking ? player->getDeck() : 0, zoneList, counterList, arrowList));
	}
	return result;
}

void Server_Game::sendGameEvent(GameEvent *event, Server_Player *exclude)
{
	event->setGameId(gameId);
	QMapIterator<int, Server_Player *> playerIterator(players);
	while (playerIterator.hasNext()) {
		Server_Player *p = playerIterator.next().value();
		if (p != exclude)
			p->sendProtocolItem(event, false);
	}

	delete event;
}
