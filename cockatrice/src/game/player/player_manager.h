/**
 * @file player_manager.h
 * @ingroup GameLogicPlayers
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_PLAYER_MANAGER_H
#define COCKATRICE_PLAYER_MANAGER_H

#include <QMap>
#include <QObject>
#include <libcockatrice/protocol/pb/serverinfo_playerproperties.pb.h>

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

    [[nodiscard]] bool isSpectator() const
    {
        return localPlayerIsSpectator;
    }

    [[nodiscard]] bool isJudge() const
    {
        return localPlayerIsJudge;
    }

    [[nodiscard]] int getLocalPlayerId() const
    {
        return localPlayerId;
    }

    [[nodiscard]] const QMap<int, Player *> &getPlayers() const
    {
        return players;
    }

    [[nodiscard]] int getPlayerCount() const
    {
        return players.size();
    }

    [[nodiscard]] Player *getActiveLocalPlayer(int activePlayer) const;
    bool isLocalPlayer(int playerId);

    Player *addPlayer(int playerId, const ServerInfo_User &info);

    void removePlayer(int playerId);

    [[nodiscard]] Player *getPlayer(int playerId) const;

    void onPlayerConceded(int playerId, bool conceded);

    [[nodiscard]] bool isMainPlayerConceded() const;

    [[nodiscard]] bool isLocalPlayer(int playerId) const
    {
        return playerId == getLocalPlayerId();
    }

    [[nodiscard]] const QMap<int, ServerInfo_User> &getSpectators() const
    {
        return spectators;
    }

    [[nodiscard]] ServerInfo_User getSpectator(int playerId) const
    {
        return spectators.value(playerId);
    }

    [[nodiscard]] QString getSpectatorName(int spectatorId) const
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

    [[nodiscard]] AbstractGame *getGame() const
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
