#include "localserver.h"
#include "localserverinterface.h"
#include "server_room.h"

LocalServer::LocalServer(QObject *parent)
    : Server(parent)
{
    setDatabaseInterface(new LocalServer_DatabaseInterface(this));
    addRoom(new Server_Room(0, 0, QString(), QString(), QString(), false, QString(), QStringList(), this));
}

LocalServer::~LocalServer()
{
    // LocalServer is single threaded so it doesn't need locks on this
    while (!clients.isEmpty())
        clients.first()->prepareDestroy();

    prepareDestroy();
}

LocalServerInterface *LocalServer::newConnection()
{
    LocalServerInterface *lsi = new LocalServerInterface(this, getDatabaseInterface());
    addClient(lsi);
    return lsi;
}

LocalServer_DatabaseInterface::LocalServer_DatabaseInterface(LocalServer *_localServer)
    : Server_DatabaseInterface(_localServer), localServer(_localServer)
{
}

ServerInfo_User LocalServer_DatabaseInterface::getUserData(const QString &name, bool /*withId*/)
{
    ServerInfo_User result;
    result.set_name(name.toStdString());
    return result;
}

AuthenticationResult LocalServer_DatabaseInterface::checkUserPassword(Server_ProtocolHandler * /* handler */, const QString & /* user */, const QString & /* password */, const QString & /* clientId */, QString & /* reasonStr */, int & /* secondsLeft */)
{
    return UnknownUser;
}
