#include "game_state.h"

GameState::GameState(int _secondsElapsed,
                     int _hostId,
                     int _localPlayerId,
                     bool _isLocalGame,
                     const QList<AbstractClient *> _clients,
                     bool _spectator,
                     bool _judge,
                     bool _gameStateKnown,
                     bool _resuming,
                     int _currentPhase,
                     bool _gameClosed)
    : secondsElapsed(_secondsElapsed), hostId(_hostId), localPlayerId(_localPlayerId), isLocalGame(_isLocalGame),
      clients(_clients), spectator(_spectator), judge(_judge), gameStateKnown(_gameStateKnown), resuming(_resuming),
      currentPhase(_currentPhase), gameClosed(_gameClosed)
{
}

void GameState::incrementGameTime()
{
    setGameTime(++secondsElapsed);
}

void GameState::setGameTime(int _secondsElapsed)
{
    int seconds = _secondsElapsed;
    int minutes = seconds / 60;
    seconds -= minutes * 60;
    int hours = minutes / 60;
    minutes -= hours * 60;

    emit updateTimeElapsedLabel(QString::number(hours).rightJustified(2, '0') + ":" +
                                QString::number(minutes).rightJustified(2, '0') + ":" +
                                QString::number(seconds).rightJustified(2, '0'));
}

void GameState::startGameTimer()
{
    gameTimer = new QTimer(this);
    gameTimer->setInterval(1000);
    connect(gameTimer, &QTimer::timeout, this, &GameState::incrementGameTime);
    gameTimer->start();
}