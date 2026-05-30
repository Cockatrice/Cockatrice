#include "player_manager.h"

#include "../abstract_game.h"
#include "player_logic.h"

PlayerManager::PlayerManager(AbstractGame *_game,
                             int _localPlayerId,
                             bool _localPlayerIsJudge,
                             bool localPlayerIsSpectator)
    : QObject(_game), game(_game), players(QMap<int, PlayerLogic *>()), localPlayerId(_localPlayerId),
      localPlayerIsJudge(_localPlayerIsJudge), localPlayerIsSpectator(localPlayerIsSpectator)
{
}

bool PlayerManager::isMainPlayerConceded() const
{
    PlayerLogic *player = players.value(localPlayerId, nullptr);
    return player && player->getConceded();
}

PlayerLogic *PlayerManager::getActiveLocalPlayer(int activePlayer) const
{
    PlayerLogic *active = players.value(activePlayer, 0);
    if (active) {
        if (active->getPlayerInfo()->getLocal()) {
            return active;
        }
    }

    QMapIterator<int, PlayerLogic *> playerIterator(players);
    while (playerIterator.hasNext()) {
        PlayerLogic *temp = playerIterator.next().value();
        if (temp->getPlayerInfo()->getLocal()) {
            return temp;
        }
    }

    return nullptr;
}

bool PlayerManager::isLocalPlayer(int playerId)
{
    return game->getGameState()->getIsLocalGame() || playerId == localPlayerId;
}

PlayerLogic *PlayerManager::addPlayer(int playerId, const ServerInfo_User &info)
{
    auto *newPlayer = new PlayerLogic(info, playerId, isLocalPlayer(playerId) || game->getGameState()->getIsLocalGame(),
                                      isJudge(), getGame());
    connect(newPlayer, &PlayerLogic::concededChanged, this, &PlayerManager::onPlayerConceded);
    players.insert(playerId, newPlayer);
    emit playerAdded(newPlayer);
    emit playerCountChanged();
    return newPlayer;
}

void PlayerManager::removePlayer(int playerId)
{
    PlayerLogic *player = getPlayer(playerId);
    if (!player) {
        return;
    }
    emit playerRemoved(player);
    emit playerCountChanged();
    players.remove(playerId);
    player->deleteLater();
}

PlayerLogic *PlayerManager::getPlayer(int playerId) const
{
    PlayerLogic *player = players.value(playerId, 0);
    if (!player) {
        return nullptr;
    }
    return player;
}

void PlayerManager::onPlayerConceded(int playerId, bool conceded)
{
    // Everything else cares about this
    if (conceded) {
        emit playerConceded(playerId);
    } else {
        emit playerUnconceded(playerId);
    }
}