#include "localserver.h"
#include "localserverinterface.h"

LocalServer::LocalServer(QObject *parent)
	: Server(parent)
{
}

LocalServer::~LocalServer()
{
}

LocalServerInterface *LocalServer::newConnection()
{
	LocalServerInterface *lsi = new LocalServerInterface(this);
	return lsi;
}
