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
	AuthenticationResult checkUserPassword(Server_ProtocolHandler * /*handler*/, const QString & /*user*/, const QString & /*password*/) { return UnknownUser; }
	QString getLoginMessage() const { return QString(); }
	bool getGameShouldPing() const { return false; }
	int getMaxGameInactivityTime() const { return 9999999; }
	int getMaxPlayerInactivityTime() const { return 9999999; }
	
	LocalServerInterface *newConnection();
protected:
	bool userExists(const QString & /*name*/) { return false; }
	ServerInfo_User *getUserData(const QString &name);
	QMap<QString, ServerInfo_User *> getBuddyList(const QString & /*name*/) { return QMap<QString, ServerInfo_User *>(); }
	QMap<QString, ServerInfo_User *> getIgnoreList(const QString & /*name*/) { return QMap<QString, ServerInfo_User *>(); }
};

#endif