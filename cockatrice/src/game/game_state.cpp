#include "game_state.h"

#include "abstract_game.h"

GameState::GameState(AbstractGame *parent,
                     int _secondsElapsed,
                     int _hostId,
                     bool _isLocalGame,
                     const QList<AbstractClient *> _clients,
                     bool _gameStateKnown,
                     bool _resuming,
                     int _currentPhase,
                     bool _gameClosed)
    : QObject(parent), gameTimer(nullptr), secondsElapsed(_secondsElapsed), hostId(_hostId), isLocalGame(_isLocalGame),
      clients(_clients), gameStateKnown(_gameStateKnown), resuming(_resuming), currentPhase(_currentPhase),
      activePlayer(-1), gameClosed(_gameClosed)
{
    gameTimer = new QTimer(this);
    gameTimer->setInterval(1000);
    connect(gameTimer, &QTimer::timeout, this, &GameState::incrementGameTime);
    gameTimer->start();
}

void GameState::incrementGameTime()
{
    setGameTime(++secondsElapsed);
}

void GameState::setGameTime(int _secondsElapsed)
{
    secondsElapsed = _secondsElapsed;
    int seconds = _secondsElapsed;
    int minutes = seconds / 60;
    seconds -= minutes * 60;
    int hours = minutes / 60;
    minutes -= hours * 60;

    emit updateTimeElapsedLabel(QString::number(hours).rightJustified(2, '0') + ":" +
                                QString::number(minutes).rightJustified(2, '0') + ":" +
                                QString::number(seconds).rightJustified(2, '0'));
}