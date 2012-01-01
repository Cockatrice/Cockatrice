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
	AuthenticationResult checkUserPassword(Server_ProtocolHandler * /*handler*/, const QString & /*user*/, const QString & /*password*/, QString & /*reasonStr*/) { return UnknownUser; }
	QString getLoginMessage() const { return QString(); }
	bool getGameShouldPing() const { return false; }
	int getMaxGameInactivityTime() const { return 9999999; }
	int getMaxPlayerInactivityTime() const { return 9999999; }
	bool getThreaded() const { return false; }
	
	LocalServerInterface *newConnection();
protected:
	int startSession(const QString & /*userName*/, const QString & /*address*/) { return -1; }
	void endSession(int /*sessionId*/) { }
	bool userExists(const QString & /*name*/) { return false; }
	ServerInfo_User getUserData(const QString &name);
	QMap<QString, ServerInfo_User> getBuddyList(const QString & /*name*/) { return QMap<QString, ServerInfo_User>(); }
	QMap<QString, ServerInfo_User> getIgnoreList(const QString & /*name*/) { return QMap<QString, ServerInfo_User>(); }
	bool isInBuddyList(const QString & /*whoseList*/, const QString & /*who*/) { return false; }
	bool isInIgnoreList(const QString & /*whoseList*/, const QString & /*who*/) { return false; }
};

#endif