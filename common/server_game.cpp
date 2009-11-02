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

Server_Game::Server_Game(const QString &_creator, int _gameId, const QString &_description, const QString &_password, int _maxPlayers, bool _spectatorsAllowed, QObject *parent)
	: QObject(parent), gameStarted(false), gameId(_gameId), description(_description), password(_password), maxPlayers(_maxPlayers), spectatorsAllowed(_spectatorsAllowed)
{
	creator = addPlayer(_creator, false);
}

Server_Game::~Server_Game()
{
	broadcastEvent("game_closed", 0);
	
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

QString Server_Game::getGameListLine() const
{
	if (players.isEmpty())
		return QString("list_games|%1|||0|%2||0|0").arg(gameId).arg(maxPlayers);
	else {
		QString creatorName = creator ? creator->getPlayerName() : QString();
		return QString("list_games|%1|%2|%3|%4|%5|%6|%7|%8").arg(gameId)
							      .arg(description)
							      .arg(password.isEmpty() ? 0 : 1)
							      .arg(players.size())
							      .arg(maxPlayers)
							      .arg(creatorName)
							      .arg(spectatorsAllowed ? 1 : 0)
							      .arg(spectators.size());
	}
}

void Server_Game::broadcastEvent(const QString &eventStr, Server_Player *player)
{
	QList<Server_Player *> allClients = QList<Server_Player *>() << players.values() << spectators;
	for (int i = 0; i < allClients.size(); ++i)
		allClients[i]->publicEvent(eventStr, player);
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
	broadcastEvent("game_start", NULL);
	setActivePlayer(0);
}

ProtocolResponse::ResponseCode Server_Game::checkJoin(const QString &_password, bool spectator)
{
	if (_password != password)
		return ProtocolResponse::RespWrongPassword;
	if (spectator) {
		if (!spectatorsAllowed)
			return ProtocolResponse::RespSpectatorsNotAllowed;
	} else if (gameStarted || (getPlayerCount() >= getMaxPlayers()))
		return ProtocolResponse::RespContextError;
	
	return ProtocolResponse::RespOk;
}

Server_Player *Server_Game::addPlayer(const QString &playerName, bool spectator)
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
	
	Server_Player *newPlayer = new Server_Player(this, playerId, playerName, spectator);
	broadcastEvent(QString("join|%1").arg(spectator ? 1 : 0), newPlayer);
	
	if (spectator)
		spectators << newPlayer;
	else
		players.insert(playerId, newPlayer);
	
	qobject_cast<Server *>(parent())->broadcastGameListUpdate(this);
	
	return newPlayer;
}

void Server_Game::removePlayer(Server_Player *player)
{
	if (player->getSpectator())
		spectators.removeAt(spectators.indexOf(player));
	else
		players.remove(player->getPlayerId());
	broadcastEvent("leave", player);
	delete player;
	
	if (!players.size())
		deleteLater();
	qobject_cast<Server *>(parent())->broadcastGameListUpdate(this);
}

void Server_Game::setActivePlayer(int _activePlayer)
{
	activePlayer = _activePlayer;
	broadcastEvent(QString("set_active_player|%1").arg(_activePlayer), NULL);
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
			broadcastEvent(QString("delete_arrow|%1").arg(a->getId()), player);
			player->deleteArrow(a->getId());
		}
	}
	
	activePhase = _activePhase;
	broadcastEvent(QString("set_active_phase|%1").arg(_activePhase), NULL);
}
