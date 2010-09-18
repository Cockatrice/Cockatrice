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
	AuthenticationResult checkUserPassword(const QString & /*user*/, const QString & /*password*/) { return UnknownUser; }
	QString getLoginMessage() const { return QString(); }
	bool getGameShouldPing() const { return false; }
	int getMaxGameInactivityTime() const { return 9999999; }
	int getMaxPlayerInactivityTime() const { return 9999999; }
	
	LocalServerInterface *newConnection();
protected:
	ServerInfo_User *getUserData(const QString &name);
};

#endif