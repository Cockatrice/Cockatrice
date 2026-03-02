#include "local_server_interface.h"

#include "local_server.h"

#include <QDebug>

LocalServerInterface::LocalServerInterface(LocalServer *_server, Server_DatabaseInterface *_databaseInterface)
    : Server_ProtocolHandler(_server, _databaseInterface, _server)
{
}

LocalServerInterface::~LocalServerInterface()
{
}

void LocalServerInterface::transmitProtocolItem(const ServerMessage &item)
{
    emit itemToClient(item);
}

void LocalServerInterface::itemFromClient(const CommandContainer &item)
{
    processCommandContainer(item);
}
