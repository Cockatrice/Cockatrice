#include "game.h"

#include "../interface/widgets/tabs/tab_game.h"

#include <libcockatrice/protocol/pb/event_game_joined.pb.h>

Game::Game(QObject *_parent,
           bool isLocalGame,
           QList<AbstractClient *> &_clients,
           const Event_GameJoined &event,
           const QMap<int, QString> &_roomGameTypes)
    : AbstractGame(_parent)
{
    gameMetaInfo->setFromProto(event.game_info());
    gameMetaInfo->setRoomGameTypes(_roomGameTypes);
    gameState = new GameState(this, 0, event.host_id(), isLocalGame, _clients, false, event.resuming(), -1, false);
    connect(gameMetaInfo, &GameMetaInfo::startedChanged, gameState, &GameState::onStartedChanged);
    playerManager = new PlayerManager(this, event.player_id(), event.judge(), event.spectator());
    gameMetaInfo->setStarted(false);
}
