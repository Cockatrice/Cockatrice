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
#ifndef SERVERGAME_H
#define SERVERGAME_H

#include <QStringList>
#include "returnmessage.h"

class ServerSocket;

class ServerGame : public QObject {
	Q_OBJECT
private:
	ServerSocket *creator;
	QList<ServerSocket *> players;
	QList<ServerSocket *> spectators;
	bool gameStarted;
	int gameId;
	QString description;
	QString password;
	int maxPlayers;
	int activePlayer, activePhase;
	bool spectatorsAllowed;
signals:
	void gameClosing();
public slots:
	void broadcastEvent(const QString &event, ServerSocket *player);
public:
	ServerGame(ServerSocket *_creator, int _gameId, const QString &_description, const QString &_password, int _maxPlayers, bool _spectatorsAllowed, QObject *parent = 0);
	~ServerGame();
	ServerSocket *getCreator() const { return creator; }
	bool getGameStarted() const { return gameStarted; }
	int getPlayerCount() const { return players.size(); }
	const QList<ServerSocket *> &getPlayers() const { return players; }
	int getGameId() const { return gameId; }
	QString getDescription() const { return description; }
	QString getPassword() const { return password; }
	int getMaxPlayers() const { return maxPlayers; }
	bool getSpectatorsAllowed() const { return spectatorsAllowed; }
	QString getGameListLine() const;
	ServerSocket *getPlayer(int playerId);
	ReturnMessage::ReturnCode checkJoin(const QString &_password, bool spectator);
	void addPlayer(ServerSocket *player, bool spectator);
	void removePlayer(ServerSocket *player);
	void startGameIfReady();
	void msg(const QString &s);
	int getActivePlayer() const { return activePlayer; }
	int getActivePhase() const { return activePhase; }
	void setActivePlayer(int _activePlayer);
	void setActivePhase(int _activePhase);
};

#endif
