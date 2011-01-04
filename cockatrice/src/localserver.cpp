#include "localserver.h"
#include "localserverinterface.h"
#include "server_room.h"

LocalServer::LocalServer(QObject *parent)
	: Server(parent)
{
	addRoom(new Server_Room(0, QString(), QString(), false, QString(), this));
}

LocalServer::~LocalServer()
{
}

LocalServerInterface *LocalServer::newConnection()
{
	LocalServerInterface *lsi = new LocalServerInterface(this);
	addClient(lsi);
	return lsi;
}

ServerInfo_User *LocalServer::getUserData(const QString &name)
{
	return new ServerInfo_User(name);
}