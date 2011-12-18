#include "localclient.h"
#include "localserverinterface.h"
#include "protocol.h"

#include "pb/session_commands.pb.h"

LocalClient::LocalClient(LocalServerInterface *_lsi, const QString &_playerName, QObject *parent)
	: AbstractClient(parent), lsi(_lsi)
{
	connect(lsi, SIGNAL(itemToClient(ProtocolItem *)), this, SLOT(itemFromServer(ProtocolItem *)));
	
	Command_Login loginCmd;
	loginCmd.set_user_name(_playerName.toStdString());
	sendCommand(prepareSessionCommand(loginCmd));
	
	Command_JoinRoom joinCmd;
	joinCmd.set_room_id(0);
	sendCommand(prepareSessionCommand(joinCmd));
}

LocalClient::~LocalClient()
{
}

void LocalClient::sendCommandContainer(const CommandContainer &cont)
{
//	cont->setReceiverMayDelete(false);
//	pendingCommands.insert(cont->getCmdId(), cont);
//	lsi->itemFromClient(cont);
}

void LocalClient::itemFromServer(ProtocolItem *item)
{
	processProtocolItem(item);
}
