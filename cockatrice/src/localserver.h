#ifndef LOCALSERVER_H
#define LOCALSERVER_H

#include "server.h"

class LocalServerInterface;

class LocalServer : public Server
{
	Q_OBJECT
public:
	LocalServer(QObject *parent = 0);
	~LocalServer();
	
	LocalServerInterface *newConnection();
protected:
	ServerInfo_User getUserData(const QString &name, bool withId = false);
};

#endif