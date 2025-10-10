#include "local_client.h"

#include "../../server/local/local_server_interface.h"

#include <libcockatrice/protocol/debug_pb_message.h>
#include <libcockatrice/protocol/pb/session_commands.pb.h>

LocalClient::LocalClient(LocalServerInterface *_lsi,
                         const QString &_playerName,
                         const QString &_clientId,
                         QObject *parent)
    : AbstractClient(parent), lsi(_lsi)
{
    connect(lsi, &LocalServerInterface::itemToClient, this, &LocalClient::itemFromServer);

    userName = _playerName;

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
    qCDebug(LocalClientLog).noquote() << userName << "OUT" << getSafeDebugString(cont);

    lsi->itemFromClient(cont);
}

void LocalClient::itemFromServer(const ServerMessage &item)
{
    qCDebug(LocalClientLog).noquote() << userName << "IN" << getSafeDebugString(item);

    processProtocolItem(item);
}
