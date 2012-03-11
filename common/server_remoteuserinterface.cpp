#include "server_remoteuserinterface.h"
#include "server.h"

void Server_RemoteUserInterface::sendProtocolItem(const Response &item)
{
	server->sendIslMessage(item, userInfo->server_id());
}

void Server_RemoteUserInterface::sendProtocolItem(const SessionEvent &item)
{
	server->sendIslMessage(item, userInfo->server_id());
}

void Server_RemoteUserInterface::sendProtocolItem(const GameEventContainer &item)
{
	server->sendIslMessage(item, userInfo->server_id());
}

void Server_RemoteUserInterface::sendProtocolItem(const RoomEvent &item)
{
	server->sendIslMessage(item, userInfo->server_id());
}
