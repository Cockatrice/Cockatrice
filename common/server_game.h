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

class QTimer;
class Server_Room;
class ServerInfo_User;

class Server_Game : public QObject {
	Q_OBJECT
private:
	ServerInfo_User *creatorInfo;
	QMap<int, Server_Player *> players;
	bool gameStarted;
	int gameId;
	QString description;
	QString password;
	int maxPlayers;
	QList<int> gameTypes;
	int activePlayer, activePhase;
	bool onlyBuddies, onlyRegistered;
	bool spectatorsAllowed;
	bool spectatorsNeedPassword;
	bool spectatorsCanTalk;
	bool spectatorsSeeEverything;
	int inactivityCounter;
	int secondsElapsed;
	QTimer *pingClock;
signals:
	void gameClosing();
private slots:
	void pingClockTimeout();
public:
	Server_Game(Server_ProtocolHandler *_creator, int _gameId, const QString &_description, const QString &_password, int _maxPlayers, const QList<int> &_gameTypes, bool _onlyBuddies, bool _onlyRegistered, bool _spectatorsAllowed, bool _spectatorsNeedPassword, bool _spectatorsCanTalk, bool _spectatorsSeeEverything, Server_Room *parent);
	~Server_Game();
	ServerInfo_Game *getInfo() const;
	ServerInfo_User *getCreatorInfo() const { return creatorInfo; }
	bool getGameStarted() const { return gameStarted; }
	int getPlayerCount() const;
	int getSpectatorCount() const;
	const QMap<int, Server_Player *> &getPlayers() const { return players; }
	Server_Player *getPlayer(int playerId) const { return players.value(playerId, 0); }
	int getGameId() const { return gameId; }
	QString getDescription() const { return description; }
	QString getPassword() const { return password; }
	int getMaxPlayers() const { return maxPlayers; }
	bool getSpectatorsAllowed() const { return spectatorsAllowed; }
	bool getSpectatorsNeedPassword() const { return spectatorsNeedPassword; }
	bool getSpectatorsCanTalk() const { return spectatorsCanTalk; }
	bool getSpectatorsSeeEverything() const { return spectatorsSeeEverything; }
	ResponseCode checkJoin(ServerInfo_User *user, const QString &_password, bool spectator);
	Server_Player *addPlayer(Server_ProtocolHandler *handler, bool spectator, bool broadcastUpdate = true);
	void removePlayer(Server_Player *player);
	void removeArrowsToPlayer(Server_Player *player);
	bool kickPlayer(int playerId);
	void startGameIfReady();
	void stopGameIfFinished();
	int getActivePlayer() const { return activePlayer; }
	int getActivePhase() const { return activePhase; }
	void setActivePlayer(int _activePlayer);
	void setActivePhase(int _activePhase);
	void nextTurn();

	QList<ServerInfo_Player *> getGameState(Server_Player *playerWhosAsking) const;
	void sendGameEvent(GameEvent *event, GameEventContext *context = 0, Server_Player *exclude = 0);
	void sendGameEventContainer(GameEventContainer *cont, Server_Player *exclude = 0, bool excludeOmniscient = false);
	void sendGameEventContainerOmniscient(GameEventContainer *cont, Server_Player *exclude = 0);
	void sendGameEventToPlayer(Server_Player *player, GameEvent *event);
};

#endif
