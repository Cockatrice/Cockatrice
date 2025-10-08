#ifndef SERVER_ABSTRACTUSERINTERFACE
#define SERVER_ABSTRACTUSERINTERFACE

#include "serverinfo_user_container.h"

#include <QMap>
#include <QMutex>
#include <QPair>
#include <libcockatrice/protocol/pb/response.pb.h>
#include <libcockatrice/protocol/pb/server_message.pb.h>

class SessionEvent;
class GameEventContainer;
class RoomEvent;
class ResponseContainer;

class Server;
class Server_Game;

class Server_AbstractUserInterface : public ServerInfo_User_Container
{
private:
    mutable QMutex gameListMutex;
    QMap<int, QPair<int, int>> games; // gameId -> (roomId, playerId)
protected:
    Server *server;

public:
    explicit Server_AbstractUserInterface(Server *_server) : server(_server)
    {
    }
    Server_AbstractUserInterface(Server *_server, const ServerInfo_User_Container &other)
        : ServerInfo_User_Container(other), server(_server)
    {
    }
    ~Server_AbstractUserInterface() override
    {
    }

    virtual int getLastCommandTime() const = 0;
    virtual bool addSaidMessageSize(int size) = 0;

    void playerRemovedFromGame(Server_Game *game);
    void playerAddedToGame(int gameId, int roomId, int playerId);
    void joinPersistentGames(ResponseContainer &rc);

    QMap<int, QPair<int, int>> getGames() const
    {
        QMutexLocker locker(&gameListMutex);
        return games;
    }

    virtual void sendProtocolItem(const Response &item) = 0;
    virtual void sendProtocolItem(const SessionEvent &item) = 0;
    virtual void sendProtocolItem(const GameEventContainer &item) = 0;
    virtual void sendProtocolItem(const RoomEvent &item) = 0;
    void sendProtocolItemByType(ServerMessage::MessageType type, const ::google::protobuf::Message &item);

    static SessionEvent *prepareSessionEvent(const ::google::protobuf::Message &sessionEvent);
    void sendResponseContainer(const ResponseContainer &responseContainer, Response::ResponseCode responseCode);
};

#endif
