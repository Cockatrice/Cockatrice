#ifndef SERVER_RESPONSE_CONTAINERS_H
#define SERVER_RESPONSE_CONTAINERS_H

#include "pb/game_event_container.pb.h"
#include "pb/server_message.pb.h"

#include <QList>
#include <QMap>
#include <QPair>

namespace google
{
namespace protobuf
{
class Message;
}
} // namespace google
class Server_Game;

class GameEventBuilder final
{
public:
    GameEventBuilder(const ::google::protobuf::Message &event, int playerId);

    GameEventBuilder withPrivateEvent(int playerId, const ::google::protobuf::Message &event);

    const GameEvent &getPublicEvent() const
    {
        return publicEvent;
    }

    const GameEvent &getReplayEvent() const
    {
        if (privatePlayerId == -1) {
            return publicEvent;
        }

        return privateEvent;
    }

    int getPrivatePlayerId() const
    {
        return privatePlayerId;
    }

    const GameEvent &getEventForPlayer(int playerId) const
    {
        if (playerId == privatePlayerId) {
            return privateEvent;
        }

        return publicEvent;
    }

private:
    GameEvent publicEvent;
    int privatePlayerId = -1;
    GameEvent privateEvent;
};

class GameEvents final
{
private:
    GameEventContainer publicEvents;
    GameEventContainer replayEvents;
    QMap<int, GameEventContainer> privateEvents;

public:
    GameEvents(const GameEventContainer &events) : publicEvents(events), replayEvents(events)
    {
    }

    GameEvents(const GameEventContainer &_publicEvents,
               const GameEventContainer &_replayEvents,
               const QMap<int, GameEventContainer> &_privateEvents)
        : publicEvents(_publicEvents), replayEvents(_replayEvents), privateEvents(_privateEvents)
    {
    }

    const GameEventContainer &getEventsForPlayer(int playerId) const;
    const GameEventContainer &getReplayEvents() const
    {
        return replayEvents;
    };

    void setGameEventContext(const GameEventContext &gameEventContext);
    void setForcedByJudge(int judgeId);
    void setGameId(int gameId);
};

class GameEventsBuilder final
{
    using Message = ::google::protobuf::Message;

private:
    QList<GameEventBuilder> gameEvents;
    QList<std::pair<QSet<QString>, QList<GameEventBuilder>>> upgrades;

public:
    void append(const GameEventBuilder &builder);

    /** Append the events in [other] to the builder, taking into consideration
     * feature sets.
     *
     * For any combination of player ID and feature sets, the events seen by
     * the corresponding player with that feature set after [append]ing is the
     * concatenation of the events seen before [append]ing and the events in
     * [other].
     * */
    void append(const GameEventsBuilder &other);

    /** Add the events in [other] as alternatives for the current set of events stored by the builder.
     *
     * Clients that have all the [features] will see the events for [other],
     * clients that lack any of the [features] will see the current events.
     *
     * Events that are added through [enqueueGameEvent] after the call to
     * [upgrade] will be seen by *all* clients, whether they have the
     * [features] or not.
     */
    void upgrade(const QSet<QString> &features, const GameEventsBuilder &other);

    bool hasEvents() const
    {
        return !gameEvents.isEmpty();
    }

    GameEvents build() const;
};

class GameEventStorage
{
private:
    GameEventContext *gameEventContext;
    GameEventsBuilder gameEventsBuilder;
    int forcedByJudge = -1;

public:
    GameEventStorage();
    ~GameEventStorage();

    void setGameEventContext(const ::google::protobuf::Message &_gameEventContext);
    void setForcedByJudge(int playerId)
    {
        forcedByJudge = playerId;
    }

    void enqueueGameEvent(const ::google::protobuf::Message &event, int playerId);
    void enqueueGameEvent(const GameEventBuilder &builder);
    void enqueueGameEvents(const GameEventsBuilder &builder);
    void sendToGame(Server_Game *game);
};

class ResponseContainer
{
private:
    int cmdId;
    ::google::protobuf::Message *responseExtension;
    QList<QPair<ServerMessage::MessageType, ::google::protobuf::Message *>> preResponseQueue, postResponseQueue;

public:
    ResponseContainer(int _cmdId);
    ~ResponseContainer();

    int getCmdId() const
    {
        return cmdId;
    }
    void setResponseExtension(::google::protobuf::Message *_responseExtension)
    {
        responseExtension = _responseExtension;
    }
    ::google::protobuf::Message *getResponseExtension() const
    {
        return responseExtension;
    }
    void enqueuePreResponseItem(ServerMessage::MessageType type, ::google::protobuf::Message *item)
    {
        preResponseQueue.append(qMakePair(type, item));
    }
    void enqueuePostResponseItem(ServerMessage::MessageType type, ::google::protobuf::Message *item)
    {
        postResponseQueue.append(qMakePair(type, item));
    }
    const QList<QPair<ServerMessage::MessageType, ::google::protobuf::Message *>> &getPreResponseQueue() const
    {
        return preResponseQueue;
    }
    const QList<QPair<ServerMessage::MessageType, ::google::protobuf::Message *>> &getPostResponseQueue() const
    {
        return postResponseQueue;
    }
};

#endif
