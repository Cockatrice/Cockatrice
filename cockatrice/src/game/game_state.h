/**
 * @file game_state.h
 * @ingroup GameLogic
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_GAME_STATE_H
#define COCKATRICE_GAME_STATE_H

#include <QObject>
#include <QTimer>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/pb/serverinfo_game.pb.h>

class AbstractGame;
class ServerInfo_PlayerProperties;
class ServerInfo_User;

class GameState : public QObject
{
    Q_OBJECT

public:
    explicit GameState(AbstractGame *parent,
                       int secondsElapsed,
                       int hostId,
                       bool isLocalGame,
                       QList<AbstractClient *> clients,
                       bool gameStateKnown,
                       bool resuming,
                       int currentPhase,
                       bool gameClosed);

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

    bool isGameStateKnown() const
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
            emit gameStarted(_started);
        } else {
            emit gameStopped();
        }
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
    int hostId;
    const bool isLocalGame;
    QList<AbstractClient *> clients;
    bool gameStateKnown;
    bool resuming;
    int currentPhase;
    int activePlayer;
    bool gameClosed;
};

#endif // COCKATRICE_GAME_STATE_H
