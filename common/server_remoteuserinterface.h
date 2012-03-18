#ifndef SERVER_REMOTEUSERINTERFACE_H
#define SERVER_REMOTEUSERINTERFACE_H

#include "server_abstractuserinterface.h"

class Server_RemoteUserInterface : public Server_AbstractUserInterface {
public:
	Server_RemoteUserInterface(Server *_server, const ServerInfo_User_Container &_userInfoContainer) : Server_AbstractUserInterface(_server, _userInfoContainer) { }
	
	int getLastCommandTime() const { return 0; }
	
	void sendProtocolItem(const Response &item);
	void sendProtocolItem(const SessionEvent &item);
	void sendProtocolItem(const GameEventContainer &item);
	void sendProtocolItem(const RoomEvent &item);
};

#endif
