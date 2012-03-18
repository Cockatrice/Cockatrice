#include "server_remoteuserinterface.h"
#include "server.h"
#include "pb/serverinfo_user.pb.h"

void Server_RemoteUserInterface::sendProtocolItem(const Response &item)
{
	server->sendIsl_Response(item, userInfo->server_id(), userInfo->session_id());
}

void Server_RemoteUserInterface::sendProtocolItem(const SessionEvent &item)
{
	server->sendIsl_SessionEvent(item, userInfo->server_id(), userInfo->session_id());
}

void Server_RemoteUserInterface::sendProtocolItem(const GameEventContainer &item)
{
	server->sendIsl_GameEventContainer(item, userInfo->server_id(), userInfo->session_id());
}

void Server_RemoteUserInterface::sendProtocolItem(const RoomEvent &item)
{
	server->sendIsl_RoomEvent(item, userInfo->server_id(), userInfo->session_id());
}
