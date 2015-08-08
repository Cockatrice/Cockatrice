#include "localclient.h"
#include "localserverinterface.h"

#include "pb/session_commands.pb.h"

LocalClient::LocalClient(LocalServerInterface *_lsi, const QString &_playerName, const QString &_clientId, QObject *parent)
    : AbstractClient(parent), lsi(_lsi)
{
    connect(lsi, SIGNAL(itemToClient(const ServerMessage &)), this, SLOT(itemFromServer(const ServerMessage &)));
    
    Command_Login loginCmd;
    loginCmd.set_user_name(_playerName.toStdString());
    loginCmd.set_clientid(_clientId.toStdString());
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
    lsi->itemFromClient(cont);
}

void LocalClient::itemFromServer(const ServerMessage &item)
{
    processProtocolItem(item);
}
