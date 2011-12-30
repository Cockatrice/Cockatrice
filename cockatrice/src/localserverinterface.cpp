#include "localserverinterface.h"
#include "localserver.h"
#include <QDebug>

LocalServerInterface::LocalServerInterface(LocalServer *_server)
	: Server_ProtocolHandler(_server, _server)
{
}

LocalServerInterface::~LocalServerInterface()
{
	prepareDestroy();
}

void LocalServerInterface::transmitProtocolItem(const ServerMessage &item)
{
	emit itemToClient(item);
}

void LocalServerInterface::itemFromClient(const CommandContainer &item)
{
	processCommandContainer(item);
}
