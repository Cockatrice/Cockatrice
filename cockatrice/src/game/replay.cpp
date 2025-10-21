#include "replay.h"

#include "../interface/widgets/tabs/tab_game.h"

Replay::Replay(TabGame *_tab, GameReplay *_replay) : AbstractGame(_tab)
{
    gameState = new GameState(this, 0, -1, tab->getTabSupervisor()->getIsLocalGame(), {}, false, false, -1, false);
    connect(gameMetaInfo, &GameMetaInfo::startedChanged, gameState, &GameState::onStartedChanged);
    playerManager = new PlayerManager(this, -1, false, true);
    loadReplay(_replay);
}
