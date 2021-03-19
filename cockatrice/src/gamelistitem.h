#ifndef GAMELISTITEM_H
#define GAMELISTITEM_H

#include "pb/serverinfo_game.pb.h"
#include "gametypemap.h"

#include <QDateTime>
#include <QObject>
#include <QVariant>

class TabRoom;


class GameListItem
{
private:
    TabRoom *room;
    ServerInfo_Game game;
    int row;
    bool full;
    QDateTime then;
    QVariant types, players, restrictions, creatorIcon, lockedIcon;
    QString spectators;
    GameTypeMap gameTypeMap;
    std::function<void()> onRemove, onUpdate;

public:
    GameListItem(TabRoom *room, const ServerInfo_Game &game, const GameTypeMap &gameTypeMap);

    const ServerInfo_Game &getGame() const {
        return game;
    }
    int getId() const {
        return game.game_id();
    }
    bool isFull() const {
        return full;
    }
    TabRoom *getRoom() const {
        return room;
    }
    bool hasPassword() const {
        return game.with_password();
    }
    bool spectatorHasPassword() const {
        return game.spectators_need_password();
    }
    int getRow() const {
        return row;
    }
    void setRow(int _row) {
        row = _row;
    }
    bool canJoinAsSpectator() const {
        return game.has_spectators_allowed();
    }
    bool canJoin() const {
        return !full;
    }

    bool update(const ServerInfo_Game &game);
    void updateFunc(std::function<void()> func);
    QVariant data(int column, int role);
};
#endif // GAMELISTITEM_H
