#include "game.h"

#include "pb/event_game_joined.pb.h"

Game::Game(QList<AbstractClient *> &_clients, const Event_GameJoined &event, const QMap<int, QString> &_roomGameTypes)
{
    gameMetaInfo = new GameMetaInfo(this);
    gameMetaInfo->setFromProto(event.game_info());
    gameMetaInfo->setRoomGameTypes(_roomGameTypes);
    gameState = new GameState(this, 0, event.host_id(), /* _tabSupervisor->getIsLocalGame() */ true, _clients, false,
                              event.resuming(), -1, false);
    playerManager = new PlayerManager(this, event.player_id(), event.spectator(), event.judge());
    gameMetaInfo->setStarted(false);

    connect(gameMetaInfo, &GameMetaInfo::startedChanged, gameState, &GameState::onStartedChanged);

    gameEventHandler = new GameEventHandler(this);

    activeCard = nullptr;
}

bool Game::isHost() const
{
    return gameState->getHostId() == playerManager->getLocalPlayerId();
}

AbstractClient *Game::getClientForPlayer(int playerId) const
{
    if (gameState->getClients().size() > 1) {
        if (playerId == -1) {
            playerId = playerManager->getActiveLocalPlayer(gameState->getActivePlayer())->getPlayerInfo()->getId();
        }

        return gameState->getClients().at(playerId);
    } else if (gameState->getClients().isEmpty())
        return nullptr;
    else
        return gameState->getClients().first();
}

void Game::loadReplay(GameReplay *replay)
{
    gameMetaInfo->setFromProto(replay->game_info());
    gameMetaInfo->setSpectatorsOmniscient(true);
}

void Game::setActiveCard(CardItem *card)
{
    activeCard = card;
}

CardItem *Game::getCard(int playerId, const QString &zoneName, int cardId) const
{
    Player *player = playerManager->getPlayer(playerId);
    if (!player)
        return nullptr;

    CardZoneLogic *zone = player->getZones().value(zoneName, 0);
    if (!zone)
        return nullptr;

    return zone->getCard(cardId);
}
