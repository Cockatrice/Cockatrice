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

#include "pb/event_leave.pb.h"
#include "pb/response.pb.h"
#include "pb/serverinfo_game.pb.h"
#include "server_response_containers.h"
#include <QDateTime>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QSet>
#include <QStringList>

class QTimer;
class GameEventContainer;
class GameReplay;
class Server_Room;
class Server_Player;
class ServerInfo_User;
class ServerInfo_Player;
class ServerInfo_Game;
class Server_AbstractUserInterface;
class Event_GameStateChanged;

class Server_Game : public QObject
{
    Q_OBJECT
private:
    Server_Room *room;
    int nextPlayerId;
    int hostId;
    ServerInfo_User *creatorInfo;
    QMap<int, Server_Player *> players;
    QSet<QString> allPlayersEver, allSpectatorsEver;
    bool gameStarted;
    bool gameClosed;
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
    int startTimeOfThisGame, secondsElapsed;
    bool firstGameStarted;
    QDateTime startTime;
    QTimer *pingClock;
    QList<GameReplay *> replayList;
    GameReplay *currentReplay;

    void createGameStateChangedEvent(Event_GameStateChanged *event,
                                     Server_Player *playerWhosAsking,
                                     bool omniscient,
                                     bool withUserInfo);
    void storeGameInformation();
signals:
    void sigStartGameIfReady();
    void gameInfoChanged(ServerInfo_Game gameInfo);
private slots:
    void pingClockTimeout();
    void doStartGameIfReady();

public:
    mutable QMutex gameMutex;
    Server_Game(const ServerInfo_User &_creatorInfo,
                int _gameId,
                const QString &_description,
                const QString &_password,
                int _maxPlayers,
                const QList<int> &_gameTypes,
                bool _onlyBuddies,
                bool _onlyRegistered,
                bool _spectatorsAllowed,
                bool _spectatorsNeedPassword,
                bool _spectatorsCanTalk,
                bool _spectatorsSeeEverything,
                Server_Room *parent);
    ~Server_Game();
    Server_Room *getRoom() const
    {
        return room;
    }
    void getInfo(ServerInfo_Game &result) const;
    int getHostId() const
    {
        return hostId;
    }
    ServerInfo_User *getCreatorInfo() const
    {
        return creatorInfo;
    }
    bool getGameStarted() const
    {
        return gameStarted;
    }
    int getPlayerCount() const;
    int getSpectatorCount() const;
    const QMap<int, Server_Player *> &getPlayers() const
    {
        return players;
    }
    int getGameId() const
    {
        return gameId;
    }
    QString getDescription() const
    {
        return description;
    }
    QString getPassword() const
    {
        return password;
    }
    int getMaxPlayers() const
    {
        return maxPlayers;
    }
    bool getSpectatorsAllowed() const
    {
        return spectatorsAllowed;
    }
    bool getSpectatorsNeedPassword() const
    {
        return spectatorsNeedPassword;
    }
    bool getSpectatorsCanTalk() const
    {
        return spectatorsCanTalk;
    }
    bool getSpectatorsSeeEverything() const
    {
        return spectatorsSeeEverything;
    }
    Response::ResponseCode
    checkJoin(ServerInfo_User *user, const QString &_password, bool spectator, bool overrideRestrictions);
    bool containsUser(const QString &userName) const;
    void addPlayer(Server_AbstractUserInterface *userInterface,
                   ResponseContainer &rc,
                   bool spectator,
                   bool broadcastUpdate = true);
    void removePlayer(Server_Player *player, Event_Leave::LeaveReason reason);
    void removeArrowsRelatedToPlayer(GameEventStorage &ges, Server_Player *player);
    void unattachCards(GameEventStorage &ges, Server_Player *player);
    bool kickPlayer(int playerId);
    void startGameIfReady();
    void stopGameIfFinished();
    int getActivePlayer() const
    {
        return activePlayer;
    }
    int getActivePhase() const
    {
        return activePhase;
    }
    void setActivePlayer(int _activePlayer);
    void setActivePhase(int _activePhase);
    void nextTurn();
    int getSecondsElapsed() const
    {
        return secondsElapsed;
    }

    void createGameJoinedEvent(Server_Player *player, ResponseContainer &rc, bool resuming);

    GameEventContainer *
    prepareGameEvent(const ::google::protobuf::Message &gameEvent, int playerId, GameEventContext *context = 0);
    GameEventContext prepareGameEventContext(const ::google::protobuf::Message &gameEventContext);

    void sendGameStateToPlayers();
    void sendGameEventContainer(GameEventContainer *cont,
                                GameEventStorageItem::EventRecipients recipients = GameEventStorageItem::SendToPrivate |
                                                                                   GameEventStorageItem::SendToOthers,
                                int privatePlayerId = -1);
};

#endif
