#include "player_manager.h"

PlayerManager::PlayerManager(QObject *parent, int _localPlayerId, bool _localPlayerIsJudge, bool localPlayerIsSpectator)
    : QObject(parent), localPlayerId(_localPlayerId), localPlayerIsJudge(_localPlayerIsJudge),
      localPlayerIsSpectator(localPlayerIsSpectator)
{
}