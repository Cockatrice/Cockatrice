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
#include "servergame.h"
#include "serversocket.h"
#include <QSqlQuery>

ServerGame::ServerGame(ServerSocket *_creator, int _gameId, const QString &_description, const QString &_password, int _maxPlayers, QObject *parent)
	: QObject(parent), creator(_creator), gameStarted(false), gameId(_gameId), description(_description), password(_password), maxPlayers(_maxPlayers)
{
}

ServerGame::~ServerGame()
{
	emit gameClosing();
	qDebug("ServerGame destructor");
}

QString ServerGame::getGameListLine() const
{
	if (players.isEmpty())
		return QString("list_games|%1|||0|%2|").arg(gameId).arg(maxPlayers);
	else
		return QString("list_games|%1|%2|%3|%4|%5|%6").arg(gameId)
							      .arg(description)
							      .arg(password.isEmpty() ? 0 : 1)
							      .arg(players.size())
							      .arg(maxPlayers)
							      .arg(creator->getPlayerName());
}

QStringList ServerGame::getPlayerNames() const
{
	QStringList result;
	QListIterator<ServerSocket *> i(players);
	while (i.hasNext()) {
		ServerSocket *tmp = i.next();
		result << QString("%1|%2").arg(tmp->getPlayerId()).arg(tmp->getPlayerName());
	}
	return result;
}

ServerSocket *ServerGame::getPlayer(int player_id)
{
	QListIterator<ServerSocket *> i(players);
	while (i.hasNext()) {
		ServerSocket *tmp = i.next();
		if (tmp->getPlayerId() == player_id)
			return tmp;
	}
	return NULL;
}

void ServerGame::msg(const QString &s)
{
	QListIterator<ServerSocket *> i(players);
	while (i.hasNext())
		i.next()->msg(s);
}

void ServerGame::broadcastEvent(const QString &cmd, ServerSocket *player)
{
	if (player)
		msg(QString("public|%1|%2|%3").arg(player->getPlayerId()).arg(player->getPlayerName()).arg(cmd));
	else
		msg(QString("public|||%1").arg(cmd));
}

void ServerGame::startGameIfReady()
{
	if (players.size() < maxPlayers)
		return;
	for (int i = 0; i < players.size(); i++)
		if (players.at(i)->getStatus() != StatusReadyStart)
			return;
	
	QSqlQuery query;
	query.prepare("insert into games (id, descr, password, time_started) values(:id, :descr, :password, now())");
	query.bindValue(":id", gameId);
	query.bindValue(":descr", description);
	query.bindValue(":password", !password.isEmpty());
	query.exec();
	
	for (int i = 0; i < players.size(); i++) {
		query.prepare("insert into games_players (id_game, player) values(:id, :player)");
		query.bindValue(":id", gameId);
		query.bindValue(":player", players.at(i)->getPlayerName());
		query.exec();
	}
	
	for (int i = 0; i < players.size(); i++)
		players.at(i)->setupZones();
	
	activePlayer = 0;
	activePhase = 0;
	gameStarted = true;
	broadcastEvent("game_start", NULL);
}

void ServerGame::addPlayer(ServerSocket *player)
{
	int max = -1;
	QListIterator<ServerSocket *> i(players);
	while (i.hasNext()) {
		int tmp = i.next()->getPlayerId();
		if (tmp > max)
			max = tmp;
	}
	player->setPlayerId(max + 1);
	
	player->setGame(this);
	player->msg(QString("private|||player_id|%1|%2").arg(max + 1).arg(player->getPlayerName()));
	broadcastEvent("join", player);
	
	players << player;
	
	connect(player, SIGNAL(broadcastEvent(const QString &, ServerSocket *)), this, SLOT(broadcastEvent(const QString &, ServerSocket *)));
}

void ServerGame::removePlayer(ServerSocket *player)
{
	players.removeAt(players.indexOf(player));
	broadcastEvent("leave", player);
	if (!players.size())
		deleteLater();
	if (!gameStarted)
		qobject_cast<Server *>(parent())->broadcastGameListUpdate(this);
}

void ServerGame::setActivePlayer(int _activePlayer)
{
	activePlayer = _activePlayer;
	broadcastEvent(QString("set_active_player|%1").arg(_activePlayer), NULL);
	setActivePhase(0);
}

void ServerGame::setActivePhase(int _activePhase)
{
	activePhase = _activePhase;
	broadcastEvent(QString("set_active_phase|%1").arg(_activePhase), NULL);
}
