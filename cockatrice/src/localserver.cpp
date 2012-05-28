#include "localserver.h"
#include "localserverinterface.h"
#include "server_room.h"

LocalServer::LocalServer(QObject *parent)
	: Server(parent)
{
	setDatabaseInterface(new LocalServer_DatabaseInterface(this));
	addRoom(new Server_Room(0, QString(), QString(), false, QString(), QStringList(), this));
}

LocalServer::~LocalServer()
{
	prepareDestroy();
}

LocalServerInterface *LocalServer::newConnection()
{
	LocalServerInterface *lsi = new LocalServerInterface(this, getDatabaseInterface());
	addClient(lsi);
	return lsi;
}

LocalServer_DatabaseInterface::LocalServer_DatabaseInterface(LocalServer *_localServer)
	: Server_DatabaseInterface(_localServer),
	  nextGameId(0),
	  nextReplayId(0)
{
}

ServerInfo_User LocalServer_DatabaseInterface::getUserData(const QString &name, bool /*withId*/)
{
	ServerInfo_User result;
	result.set_name(name.toStdString());
	return result;
}

AuthenticationResult LocalServer_DatabaseInterface::checkUserPassword(Server_ProtocolHandler *handler, const QString &user, const QString &password, QString &reasonStr, int &secondsLeft)
{
	return UnknownUser;
}
