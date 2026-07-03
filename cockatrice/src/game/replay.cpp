#include "replay.h"

#include "../interface/widgets/tabs/tab_game.h"

Replay::Replay(QObject *_parent, GameReplay *_replay, bool isLocalGame) : AbstractGame(_parent)
{
    gameState = new GameState(this, 0, -1, isLocalGame, {}, false, false, -1, false);
    connect(gameMetaInfo, &GameMetaInfo::startedChanged, gameState, &GameState::onStartedChanged);
    playerManager = new PlayerManager(this, -1, false, true);
    loadReplay(_replay);
}
