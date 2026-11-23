#ifndef SERVER_REMOTEUSERINTERFACE_H
#define SERVER_REMOTEUSERINTERFACE_H

#include "server_abstractuserinterface.h"

class Server;
class ServerInfo_User_Container;

class Server_RemoteUserInterface : public Server_AbstractUserInterface
{
public:
    Server_RemoteUserInterface(Server *_server, const ServerInfo_User_Container &_userInfoContainer)
        : Server_AbstractUserInterface(_server, _userInfoContainer)
    {
    }

    int getLastCommandTime() const override
    {
        return 0;
    }
    bool addSaidMessageSize(int /*size*/) override
    {
        return true;
    }

    void sendProtocolItem(const Response &item) override;
    void sendProtocolItem(const SessionEvent &item) override;
    void sendProtocolItem(const GameEventContainer &item) override;
    void sendProtocolItem(const RoomEvent &item) override;
};

#endif
