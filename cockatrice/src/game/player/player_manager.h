#ifndef COCKATRICE_PLAYER_MANAGER_H
#define COCKATRICE_PLAYER_MANAGER_H

#include "player.h"

#include <QObject>

class PlayerManager : public QObject
{
    Q_OBJECT

public:
    PlayerManager(QObject *parent, int _localPlayerId, bool _localPlayerIsJudge, bool localPlayerIsSpectator);

    QMap<int, Player *> players;
    int localPlayerId;
    bool localPlayerIsJudge;
    bool localPlayerIsSpectator;

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

    Player *getActiveLocalPlayer(int activePlayer) const
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

    Player *addPlayer(int playerId, const ServerInfo_User &info, TabGame *game)
    {
        auto *newPlayer = new Player(info, playerId, isLocalPlayer(playerId), isJudge(), game);
        // TODO
        // connect(newPlayer, &Player::openDeckEditor, game, &TabGame::openDeckEditor);
        players.insert(playerId, newPlayer);
        emit playerAdded(newPlayer);
        emit playerCountChanged();
        return newPlayer;
    }

    void removePlayer(int playerId)
    {
        Player *player = getPlayer(playerId);
        if (!player) {
            return;
        }
        players.remove(playerId);
        emit playerCountChanged();
        emit playerRemoved(player);
    }

    Player *getPlayer(int playerId) const
    {
        Player *player = players.value(playerId, 0);
        if (!player)
            return nullptr;
        return player;
    }

    [[nodiscard]] bool isMainPlayerConceded() const
    {
        Player *player = players.value(localPlayerId, nullptr);
        return player && player->getPlayerInfo()->getConceded();
    }

    [[nodiscard]] bool isLocalPlayer(int playerId) const
    {
        return playerId == getLocalPlayerId();
    }

signals:
    void playerAdded(Player *player);
    void playerRemoved(Player *player);
    void playerCountChanged();
};

#endif // COCKATRICE_PLAYER_MANAGER_H
