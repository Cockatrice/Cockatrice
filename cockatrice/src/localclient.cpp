#include "localclient.h"
#include "localserverinterface.h"
#include "protocol.h"

LocalClient::LocalClient(LocalServerInterface *_lsi, const QString &_playerName, QObject *parent)
	: AbstractClient(parent), lsi(_lsi)
{
	connect(lsi, SIGNAL(itemToClient(ProtocolItem *)), this, SLOT(itemFromServer(ProtocolItem *)));
	sendCommand(new Command_Login(_playerName, QString()));
	sendCommand(new Command_JoinRoom(0));
}

LocalClient::~LocalClient()
{
}

void LocalClient::sendCommandContainer(CommandContainer *cont)
{
	cont->setReceiverMayDelete(false);
	pendingCommands.insert(cont->getCmdId(), cont);
	lsi->itemFromClient(cont);
}

void LocalClient::itemFromServer(ProtocolItem *item)
{
	processProtocolItem(item);
}
