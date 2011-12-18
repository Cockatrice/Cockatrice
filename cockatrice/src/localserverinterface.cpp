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

void LocalServerInterface::sendProtocolItem(ProtocolItem *item, bool deleteItem)
{
	item->setReceiverMayDelete(false);
	emit itemToClient(item);
	if (deleteItem)
		delete item;
}
#include "pb/commands.pb.h"
void LocalServerInterface::itemFromClient(const CommandContainer &item)
{
	qDebug() << "READ" << QString::fromStdString(item.ShortDebugString());
	processCommandContainer(item);
}
