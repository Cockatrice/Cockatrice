/**
 * @file game_meta_info.h
 * @ingroup GameLogic
 * @brief TODO: Document this.
 */

#ifndef GAME_META_INFO_H
#define GAME_META_INFO_H

#include <QMap>
#include <QObject>
#include <libcockatrice/protocol/pb/serverinfo_game.pb.h>

// Translation layer class to expose protobuf safely and hook it up to Qt Signals.
// This class de-couples the domain object (i.e. the GameMetaInfo) from the network object.
// If the network object changes, only this class needs to be adjusted.

class AbstractGame;
class GameMetaInfo : public QObject
{
    Q_OBJECT
public:
    explicit GameMetaInfo(AbstractGame *parent);

    QMap<int, QString> roomGameTypes;

    // Populate from protobuf (e.g., after network message)
    void setFromProto(const ServerInfo_Game &gi)
    {
        gameInfo_.CopyFrom(gi);
    }

    const ServerInfo_Game &proto() const
    {
        return gameInfo_;
    }

    // High-level getters that avoid exposing protobuf directly
    int gameId() const
    {
        return gameInfo_.game_id();
    }
    int maxPlayers() const
    {
        return gameInfo_.max_players();
    }
    QString description() const
    {
        return QString::fromStdString(gameInfo_.description());
    }
    bool started() const
    {
        return gameInfo_.started();
    }
    bool spectatorsOmniscient() const
    {
        return gameInfo_.spectators_omniscient();
    }
    bool spectatorsCanChat() const
    {
        return gameInfo_.spectators_can_chat();
    }
    int gameTypesSize() const
    {
        return gameInfo_.game_types_size();
    }
    int gameTypeIdAt(int index) const
    {
        return gameInfo_.game_types(index);
    }

    QMap<int, QString> getRoomGameTypes() const
    {
        return roomGameTypes;
    }

    void setRoomGameTypes(QMap<int, QString> _roomGameTypes)
    {
        roomGameTypes = _roomGameTypes;
    }

    QString findRoomGameType(int index)
    {
        return roomGameTypes.find(gameInfo_.game_types(index)).value();
    }

public slots:
    void setStarted(bool s)
    {
        if (gameInfo_.started() == s)
            return;
        gameInfo_.set_started(s);
        emit startedChanged(s);
    }
    void setSpectatorsOmniscient(bool v)
    {
        if (gameInfo_.spectators_omniscient() == v)
            return;
        gameInfo_.set_spectators_omniscient(v);
        emit spectatorsOmniscienceChanged(v);
    }

signals:
    void startedChanged(bool started);
    void spectatorsOmniscienceChanged(bool omniscient);

private:
    ServerInfo_Game gameInfo_;
};

#endif // GAME_META_INFO_H
