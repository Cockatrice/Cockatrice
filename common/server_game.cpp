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
#include <QSqlQuery>

Server_Game::Server_Game(Server_ProtocolHandler *_creator, int _gameId, const QString &_description, const QString &_password, int _maxPlayers, bool _spectatorsAllowed, QObject *parent)
	: QObject(parent), gameStarted(false), gameId(_gameId), description(_description), password(_password), maxPlayers(_maxPlayers), spectatorsAllowed(_spectatorsAllowed)
{
	creator = addPlayer(_creator, false, false);
}

Server_Game::~Server_Game()
{
	sendGameEvent(new Event_GameClosed);
	
	QMapIterator<int, Server_Player *> playerIterator(players);
	while (playerIterator.hasNext())
		delete playerIterator.next().value();
	players.clear();
	for (int i = 0; i < spectators.size(); ++i)
		delete spectators[i];
	spectators.clear();
	
	emit gameClosing();
	qDebug("Server_Game destructor");
}

void Server_Game::startGameIfReady()
{
	if (players.size() < maxPlayers)
		return;
	QMapIterator<int, Server_Player *> playerIterator(players);
	while (playerIterator.hasNext())
		if (playerIterator.next().value()->getStatus() != StatusReadyStart)
			return;
	
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

		player->setupZones();
	}
*/	
	gameStarted = true;
	sendGameEvent(new Event_GameStart);
	setActivePlayer(0);
}

ResponseCode Server_Game::checkJoin(const QString &_password, bool spectator)
{
	if (_password != password)
		return RespWrongPassword;
	if (spectator) {
		if (!spectatorsAllowed)
			return RespSpectatorsNotAllowed;
	} else if (gameStarted || (getPlayerCount() >= getMaxPlayers()))
		return RespContextError;
	
	return RespOk;
}

Server_Player *Server_Game::addPlayer(Server_ProtocolHandler *handler, bool spectator, bool broadcastUpdate)
{
	int playerId;
	if (!spectator) {
		int max = -1;
		QMapIterator<int, Server_Player *> i(players);
		while (i.hasNext()) {
			int tmp = i.next().value()->getPlayerId();
			if (tmp > max)
				max = tmp;
		}
		playerId = max + 1;
	} else
		playerId = -1;
	
	Server_Player *newPlayer = new Server_Player(this, playerId, handler->getPlayerName(), spectator, handler);
	sendGameEvent(new Event_Join(-1, playerId, handler->getPlayerName(), spectator));
	
	if (spectator)
		spectators << newPlayer;
	else
		players.insert(playerId, newPlayer);
	
	if (broadcastUpdate)
		qobject_cast<Server *>(parent())->broadcastGameListUpdate(this);
	
	return newPlayer;
}

void Server_Game::removePlayer(Server_Player *player)
{
	if (player->getSpectator())
		spectators.removeAt(spectators.indexOf(player));
	else
		players.remove(player->getPlayerId());
	sendGameEvent(new Event_Leave(-1, player->getPlayerId()));
	delete player;
	
	if (!players.size())
		deleteLater();
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

void Server_Game::sendGameEvent(GameEvent *event)
{
	event->setGameId(gameId);
	QList<Server_Player *> receivers = QList<Server_Player *>() << players.values() << spectators;
	
	for (int i = 0; i < receivers.size(); ++i)
		receivers[i]->sendProtocolItem(event, false);

	delete event;
}
