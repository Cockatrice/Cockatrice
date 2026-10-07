#include "replay.h"

#include "game_meta_info.h"
#include "game_state.h"
#include "player/player_manager.h"

#include <QList>

class GameReplay;
class QObject;

Replay::Replay(QObject *_parent, const GameReplay *_replay, bool isLocalGame) : AbstractGame(_parent)
{
    gameState = new GameState(this, 0, -1, isLocalGame, {}, false, false, -1, false);
    connect(gameMetaInfo, &GameMetaInfo::startedChanged, gameState, &GameState::onStartedChanged);
    playerManager = new PlayerManager(this, -1, false, true);
    loadReplay(_replay);
}
