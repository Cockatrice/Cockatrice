#include "localclient.h"
#include "localserverinterface.h"
#include "protocol.h"

LocalClient::LocalClient(LocalServerInterface *_lsi, QObject *parent)
	: AbstractClient(parent), lsi(_lsi)
{
	connect(lsi, SIGNAL(itemToClient(ProtocolItem *)), this, SLOT(itemFromServer(ProtocolItem *)));
}

LocalClient::~LocalClient()
{
}

void LocalClient::sendCommandContainer(CommandContainer *cont)
{
	lsi->itemFromClient(cont);
}

void LocalClient::itemFromServer(ProtocolItem *item)
{
	processProtocolItem(item);
}
