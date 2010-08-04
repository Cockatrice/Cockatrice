#include "localserverinterface.h"
#include "localserver.h"

LocalServerInterface::LocalServerInterface(LocalServer *_server)
	: Server_ProtocolHandler(_server, _server)
{
}

LocalServerInterface::~LocalServerInterface()
{
}

bool LocalServerInterface::sendProtocolItem(ProtocolItem *item, bool deleteItem)
{
	emit itemToClient(item);
	return false;
}

void LocalServerInterface::itemFromClient(ProtocolItem *item)
{
	processCommandContainer(static_cast<CommandContainer *>(item));
}
