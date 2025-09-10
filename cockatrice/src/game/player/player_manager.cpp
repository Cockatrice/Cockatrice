#include "player_manager.h"

#include "../abstract_game.h"
#include "player.h"

PlayerManager::PlayerManager(AbstractGame *_game, int _localPlayerId, bool _localPlayerIsJudge, bool localPlayerIsSpectator)
    : QObject(_game), game(_game), players(QMap<int, Player *>()), localPlayerId(_localPlayerId),
      localPlayerIsJudge(_localPlayerIsJudge), localPlayerIsSpectator(localPlayerIsSpectator)
{
}

bool PlayerManager::isMainPlayerConceded() const
{
    Player *player = players.value(localPlayerId, nullptr);
    return player && player->getConceded();
}

Player *PlayerManager::getActiveLocalPlayer(int activePlayer) const
{
    Player *active = players.value(activePlayer, 0);
    if (active)
        if (active->getPlayerInfo()->getLocal())
            return active;

    QMapIterator<int, Player *> playerIterator(players);
    while (playerIterator.hasNext()) {
        Player *temp = playerIterator.next().value();
        if (temp->getPlayerInfo()->getLocal())
            return temp;
    }

    return nullptr;
}

Player *PlayerManager::addPlayer(int playerId, const ServerInfo_User &info)
{
    auto *newPlayer = new Player(info, playerId, isLocalPlayer(playerId) || game->getGameState()->getIsLocalGame(),
                                 isJudge(), getGame());
    connect(newPlayer, &Player::concededChanged, this, &PlayerManager::playerConceded);
    players.insert(playerId, newPlayer);
    emit playerAdded(newPlayer);
    emit playerCountChanged();
    return newPlayer;
}

void PlayerManager::removePlayer(int playerId)
{
    Player *player = getPlayer(playerId);
    if (!player) {
        return;
    }
    players.remove(playerId);
    emit playerCountChanged();
    emit playerRemoved(player);
}

Player *PlayerManager::getPlayer(int playerId) const
{
    Player *player = players.value(playerId, 0);
    if (!player)
        return nullptr;
    return player;
}

void PlayerManager::onPlayerConceded(int playerId, bool conceded)
{
    // GameEventHandler cares about this for sending the concede/unconcede commands
    if (playerId == getActiveLocalPlayer(playerId)->getPlayerInfo()->getId()) {
        if (conceded) {
            emit activeLocalPlayerConceded();
        } else {
            emit activeLocalPlayerUnconceded();
        }
    }
    // Everything else cares about this
    if (conceded) {
        emit playerConceded(playerId);
    } else {
        emit playerUnconceded(playerId);
    }
}