#ifndef COCKATRICE_GAME_STATE_H
#define COCKATRICE_GAME_STATE_H

#include "../client/tabs/tab_game.h"
#include "../server/abstract_client.h"
#include "pb/serverinfo_game.pb.h"
#include "pb/serverinfo_playerproperties.pb.h"
#include "player/player.h"

#include <QObject>

class ServerInfo_PlayerProperties;
class ServerInfo_User;

class GameState : public QObject
{
    Q_OBJECT

public:
    explicit GameState(int secondsElapsed,
                       int hostId,
                       int localPlayerId,
                       bool isLocalGame,
                       QList<AbstractClient *> clients,
                       bool spectator,
                       bool judge,
                       bool gameStateKnown,
                       bool resuming,
                       int currentPhase,
                       bool gameClosed);

    const QMap<int, Player *> &getPlayers() const
    {
        return players;
    }

    int getPlayerCount() const
    {
        return players.size();
    }

    int getMaxPlayerCount() const
    {
        return gameInfo.max_players();
    }

    const QMap<int, ServerInfo_User> &getSpectators() const
    {
        return spectators;
    }

    ServerInfo_User getSpectator(int playerId) const
    {
        return spectators.value(playerId);
    }

    QString getSpectatorName(int spectatorId) const
    {
        return QString::fromStdString(spectators.value(spectatorId).name());
    }

    void addSpectator(int spectatorId, const ServerInfo_PlayerProperties &prop)
    {
        if (!spectators.contains(spectatorId)) {
            spectators.insert(spectatorId, prop.user_info());
            emit spectatorAdded(prop);
        }
    }

    void removeSpectator(int spectatorId)
    {
        ServerInfo_User spectatorInfo = spectators.value(spectatorId);
        spectators.remove(spectatorId);
        emit spectatorRemoved(spectatorId, spectatorInfo);
    }

    bool isHost() const
    {
        return hostId == localPlayerId;
    }

    void setHostId(int _hostId)
    {
        hostId = _hostId;
    }

    bool isJudge() const
    {
        return judge;
    }

    int getLocalPlayerId() const
    {
        return localPlayerId;
    }

    QList<AbstractClient *> getClients() const
    {
        return clients;
    }

    bool isLocalPlayer(int playerId) const
    {
        return clients.size() > 1 || playerId == getLocalPlayerId();
    }

    Player *addPlayer(int playerId, const ServerInfo_User &info, TabGame *game)
    {
        auto *newPlayer = new Player(info, playerId, isLocalPlayer(playerId), isJudge(), game);
        // TODO
        // connect(newPlayer, &Player::openDeckEditor, game, &TabGame::openDeckEditor);
        players.insert(playerId, newPlayer);
        emit playerAdded(newPlayer);
        return newPlayer;
    }

    void removePlayer(int playerId)
    {
        Player *player = getPlayer(playerId);
        if (!player) {
            return;
        }
        players.remove(playerId);
        emit playerRemoved(player);
    }

    Player *getPlayer(int playerId)
    {
        Player *player = players.value(playerId, 0);
        if (!player)
            return nullptr;
        return player;
    }

    Player *getActiveLocalPlayer() const
    {
        Player *active = players.value(activePlayer, 0);
        if (active)
            if (active->getLocal())
                return active;

        QMapIterator<int, Player *> playerIterator(players);
        while (playerIterator.hasNext()) {
            Player *temp = playerIterator.next().value();
            if (temp->getLocal())
                return temp;
        }

        return nullptr;
    }

    void setActivePlayer(int activePlayerId)
    {
        activePlayer = activePlayerId;
    }

    bool getIsLocalGame() const
    {
        return isLocalGame;
    }

    int getGameId() const
    {
        return gameInfo.game_id();
    }

    QString getGameDescription() const
    {
        return gameInfo.description().c_str();
    }

    bool isSpectator() const
    {
        return spectator;
    }

    bool isSpectatorsOmniscient() const
    {
        return gameInfo.spectators_omniscient();
    }

    bool canSpectatorsChat() const
    {
        return gameInfo.spectators_can_chat();
    }

    bool isResuming() const
    {
        return resuming;
    }

    void setStarted(bool _started)
    {
        gameInfo.set_started(_started);
        if (_started) {
            emit gameStarted(resuming);
        } else {
            emit gameStopped();
        }
    }

    void setResuming(bool _resuming)
    {
        resuming = _resuming;
    }

    bool isGameStateKnown()
    {
        return gameStateKnown;
    }

    int getCurrentPhase() const
    {
        return currentPhase;
    }

    void setCurrentPhase(int phase)
    {
        currentPhase = phase;
    }

    bool isMainPlayerConceded() const
    {
        Player *player = players.value(localPlayerId, nullptr);
        return player && player->getConceded();
    }

    void setGameClosed(bool closed)
    {
        gameClosed = closed;
    }

    bool isGameClosed() const
    {
        return gameClosed;
    }

    void startGameTimer();

    ServerInfo_Game getGameInfo() const
    {
        return gameInfo;
    }

    void setGameInfo(ServerInfo_Game _gameInfo)
    {
        gameInfo.CopyFrom(_gameInfo);
    }

    QMap<int, QString> getRoomGameTypes() const
    {
        return roomGameTypes;
    }

    void setRoomGameTypes(QMap<int, QString> _roomGameTypes)
    {
        roomGameTypes = _roomGameTypes;
    }

    int getGameTypesSize() const
    {
        return gameInfo.game_types_size();
    }

    QString findRoomGameType(int index)
    {
        return roomGameTypes.find(gameInfo.game_types(index)).value();
    }

    void setSpectatorsOmniscient(bool spectatorsOmniscient)
    {
        gameInfo.set_spectators_omniscient(spectatorsOmniscient);
    }

    void setGameStateKnown(bool known)
    {
        gameStateKnown = known;
    }

signals:
    void updateTimeElapsedLabel(QString newTime);
    void playerAdded(Player *player);
    void playerRemoved(Player *player);
    void spectatorAdded(ServerInfo_PlayerProperties spectator);
    void spectatorRemoved(int spectatorId, ServerInfo_User spectator);
    void gameStarted(bool resuming);
    void gameStopped();

public slots:
    void incrementGameTime();
    void setGameTime(int _secondsElapsed);

private:
    QTimer *gameTimer;
    int secondsElapsed;
    ServerInfo_Game gameInfo;
    QMap<int, QString> roomGameTypes;
    int hostId;
    int localPlayerId;
    const bool isLocalGame;
    QMap<int, Player *> players;
    QMap<int, ServerInfo_User> spectators;
    QList<AbstractClient *> clients;
    bool spectator;
    bool judge;
    bool gameStateKnown;
    bool resuming;
    QStringList phasesList;
    int currentPhase;
    int activePlayer;
    bool gameClosed;
};

#endif // COCKATRICE_GAME_STATE_H
