#ifndef LOCALSERVER_H
#define LOCALSERVER_H

#include "server.h"
#include "server_database_interface.h"

class LocalServerInterface;

class LocalServer : public Server
{
    Q_OBJECT
public:
    LocalServer(QObject *parent = 0);
    ~LocalServer();
    
    LocalServerInterface *newConnection();
};

class LocalServer_DatabaseInterface : public Server_DatabaseInterface {
    Q_OBJECT
private:
    LocalServer *localServer;
protected:
    ServerInfo_User getUserData(const QString &name, bool withId = false);
public:
    LocalServer_DatabaseInterface(LocalServer *_localServer);
    AuthenticationResult checkUserPassword(Server_ProtocolHandler *handler, const QString &user, const QString &password, const QString &clientId, QString &reasonStr, int &secondsLeft);
    int getNextGameId() { return localServer->getNextLocalGameId(); }
    int getNextReplayId() { return -1; }
    int getActiveUserCount(QString /* connectionType */) { return 0; }
};

#endif
