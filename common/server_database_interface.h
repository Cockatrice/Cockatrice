#ifndef SERVER_DATABASE_INTERFACE_H
#define SERVER_DATABASE_INTERFACE_H

#include <QObject>

#include "server.h"

class Server_DatabaseInterface : public QObject {
	Q_OBJECT
private:
	int nextGameId, nextReplayId;
public:
	Server_DatabaseInterface();
	
	virtual AuthenticationResult checkUserPassword(Server_ProtocolHandler *handler, const QString &user, const QString &password, QString &reasonStr, int &secondsLeft) = 0;
	virtual bool userExists(const QString &user) { return false; }
	virtual QMap<QString, ServerInfo_User> getBuddyList(const QString &name) { return QMap<QString, ServerInfo_User>(); }
	virtual QMap<QString, ServerInfo_User> getIgnoreList(const QString &name) { return QMap<QString, ServerInfo_User>(); }
	virtual bool isInBuddyList(const QString &whoseList, const QString &who) { return false; }
	virtual bool isInIgnoreList(const QString &whoseList, const QString &who) { return false; }
	virtual ServerInfo_User getUserData(const QString &name, bool withId = false) = 0;
	
	virtual qint64 startSession(const QString &userName, const QString &address) { return 0; }
public slots:
	virtual void endSession(qint64 sessionId) { }
public:
	virtual int getNextGameId() { return nextGameId++; }
	virtual int getNextReplayId() { return nextReplayId++; }
	
	virtual void clearSessionTables() { }
	virtual void lockSessionTables() { }
	virtual void unlockSessionTables() { }
	virtual bool userSessionExists(const QString &userName) { return false; }
};

#endif
