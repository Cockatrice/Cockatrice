#include "localserverinterface.h"
#include "localserver.h"
#include <QDebug>

LocalServerInterface::LocalServerInterface(LocalServer *_server)
	: Server_ProtocolHandler(_server, _server)
{
}

LocalServerInterface::~LocalServerInterface()
{
}

void LocalServerInterface::sendProtocolItem(ProtocolItem *item, bool deleteItem)
{
	item->setReceiverMayDelete(false);
	emit itemToClient(item);
	if (deleteItem)
		delete item;
}

void LocalServerInterface::itemFromClient(ProtocolItem *item)
{
	processCommandContainer(static_cast<CommandContainer *>(item));
}
