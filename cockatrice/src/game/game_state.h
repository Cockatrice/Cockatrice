#ifndef COCKATRICE_GAME_STATE_H
#define COCKATRICE_GAME_STATE_H

#include "../client/tabs/tab_game.h"
#include "../server/abstract_client.h"
#include "pb/serverinfo_game.pb.h"
#include "pb/serverinfo_playerproperties.pb.h"

#include <QObject>

class ServerInfo_PlayerProperties;
class ServerInfo_User;

class GameState : public QObject
{
    Q_OBJECT

public:
    explicit GameState(int secondsElapsed,
                       int hostId,
                       bool isLocalGame,
                       QList<AbstractClient *> clients,
                       bool gameStateKnown,
                       bool resuming,
                       int currentPhase,
                       bool gameClosed);

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

    void setHostId(int _hostId)
    {
        hostId = _hostId;
    }

    QList<AbstractClient *> getClients() const
    {
        return clients;
    }

    bool getIsLocalGame() const
    {
        return isLocalGame;
    }

    bool isResuming() const
    {
        return resuming;
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
        emit activePhaseChanged(phase);
    }

    void setActivePlayer(int activePlayerId)
    {
        activePlayer = activePlayerId;
        emit activePlayerChanged(activePlayer);
    }

    int getActivePlayer() const
    {
        return activePlayer;
    }

    void setGameClosed(bool closed)
    {
        gameClosed = closed;
    }

    bool isGameClosed() const
    {
        return gameClosed;
    }

    void onStartedChanged(bool _started)
    {
        if (_started) {
            startGameTimer();
            emit gameStarted(_started);
        } else {
            emit gameStopped();
        }
    }

    void startGameTimer();

    QMap<int, QString> getRoomGameTypes() const
    {
        return roomGameTypes;
    }

    void setRoomGameTypes(QMap<int, QString> _roomGameTypes)
    {
        roomGameTypes = _roomGameTypes;
    }

    void setGameStateKnown(bool known)
    {
        gameStateKnown = known;
    }

    int getHostId() const
    {
        return hostId;
    }

signals:
    void updateTimeElapsedLabel(QString newTime);
    void playerAdded(Player *player);
    void playerRemoved(Player *player);
    void spectatorAdded(ServerInfo_PlayerProperties spectator);
    void spectatorRemoved(int spectatorId, ServerInfo_User spectator);
    void gameStarted(bool resuming);
    void gameStopped();
    void activePhaseChanged(int activePhase);
    void activePlayerChanged(int playerId);

public slots:
    void incrementGameTime();
    void setGameTime(int _secondsElapsed);

private:
    QTimer *gameTimer;
    int secondsElapsed;
    QMap<int, QString> roomGameTypes;
    int hostId;
    const bool isLocalGame;
    QMap<int, ServerInfo_User> spectators;
    QList<AbstractClient *> clients;
    bool gameStateKnown;
    bool resuming;
    QStringList phasesList;
    int currentPhase;
    int activePlayer;
    bool gameClosed;
};

#endif // COCKATRICE_GAME_STATE_H
