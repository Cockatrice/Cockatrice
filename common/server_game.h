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
#include <QPointer>
#include <QObject>
#include "server_player.h"
#include "protocol.h"

class Server_Game : public QObject {
	Q_OBJECT
private:
	QPointer<Server_Player> creator;
	QMap<int, Server_Player *> players;
	QList<Server_Player *> spectators;
	bool gameStarted;
	int gameId;
	QString description;
	QString password;
	int maxPlayers;
	int activePlayer, activePhase;
	bool spectatorsAllowed;
signals:
	void gameClosing();
public:
	Server_Game(Server_ProtocolHandler *_creator, int _gameId, const QString &_description, const QString &_password, int _maxPlayers, bool _spectatorsAllowed, QObject *parent = 0);
	~Server_Game();
	Server_Player *getCreator() const { return creator; }
	QString getCreatorName() const { return creator ? creator->getPlayerName() : QString(); }
	bool getGameStarted() const { return gameStarted; }
	int getPlayerCount() const { return players.size(); }
	int getSpectatorCount() const { return spectators.size(); }
	QList<Server_Player *> getPlayers() const { return players.values(); }
	Server_Player *getPlayer(int playerId) const { return players.value(playerId, 0); }
	int getGameId() const { return gameId; }
	QString getDescription() const { return description; }
	QString getPassword() const { return password; }
	int getMaxPlayers() const { return maxPlayers; }
	bool getSpectatorsAllowed() const { return spectatorsAllowed; }
	ResponseCode checkJoin(const QString &_password, bool spectator);
	Server_Player *addPlayer(Server_ProtocolHandler *handler, bool spectator);
	void removePlayer(Server_Player *player);
	void startGameIfReady();
	int getActivePlayer() const { return activePlayer; }
	int getActivePhase() const { return activePhase; }
	void setActivePlayer(int _activePlayer);
	void setActivePhase(int _activePhase);

	void sendGameEvent(GameEvent *event);
};

#endif
