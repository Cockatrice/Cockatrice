/**
 * @file player_manager.h
 * @ingroup GameLogicPlayers
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_PLAYER_MANAGER_H
#define COCKATRICE_PLAYER_MANAGER_H

#include "pb/serverinfo_playerproperties.pb.h"

#include <QMap>
#include <QObject>

class AbstractGame;
class Player;
class PlayerManager : public QObject
{
    Q_OBJECT

public:
    PlayerManager(AbstractGame *_game, int _localPlayerId, bool _localPlayerIsJudge, bool localPlayerIsSpectator);

    AbstractGame *game;
    QMap<int, Player *> players;
    int localPlayerId;
    bool localPlayerIsJudge;
    bool localPlayerIsSpectator;
    QMap<int, ServerInfo_User> spectators;

    bool isSpectator() const
    {
        return localPlayerIsSpectator;
    }

    bool isJudge() const
    {
        return localPlayerIsJudge;
    }

    int getLocalPlayerId() const
    {
        return localPlayerId;
    }

    const QMap<int, Player *> &getPlayers() const
    {
        return players;
    }

    int getPlayerCount() const
    {
        return players.size();
    }

    Player *getActiveLocalPlayer(int activePlayer) const;
    bool isLocalPlayer(int playerId);

    Player *addPlayer(int playerId, const ServerInfo_User &info);

    void removePlayer(int playerId);

    Player *getPlayer(int playerId) const;

    void onPlayerConceded(int playerId, bool conceded);

    [[nodiscard]] bool isMainPlayerConceded() const;

    [[nodiscard]] bool isLocalPlayer(int playerId) const
    {
        return playerId == getLocalPlayerId();
    }

    const QMap<int, ServerInfo_User> &getSpectators() const
    {
        return spectators;
    }

    ServerInfo_User getSpectator(int playerId) const
    {
        return spectators.value(playerId);
    }

    QString getSpectatorName(int spectatorId) const
    {
        return QString::fromStdString(spectators.value(spectatorId).name());
    }

    void addSpectator(int spectatorId, const ServerInfo_PlayerProperties &prop)
    {
        if (!spectators.contains(spectatorId)) {
            spectators.insert(spectatorId, prop.user_info());
            emit spectatorAdded(prop);
        }
    }

    void removeSpectator(int spectatorId)
    {
        ServerInfo_User spectatorInfo = spectators.value(spectatorId);
        spectators.remove(spectatorId);
        emit spectatorRemoved(spectatorId, spectatorInfo);
    }

    AbstractGame *getGame() const
    {
        return game;
    }

signals:
    void playerAdded(Player *player);
    void playerRemoved(Player *player);
    void activeLocalPlayerConceded();
    void activeLocalPlayerUnconceded();
    void playerConceded(int playerId);
    void playerUnconceded(int playerId);
    void playerCountChanged();
    void spectatorAdded(ServerInfo_PlayerProperties spectator);
    void spectatorRemoved(int spectatorId, ServerInfo_User spectator);
};

#endif // COCKATRICE_PLAYER_MANAGER_H
