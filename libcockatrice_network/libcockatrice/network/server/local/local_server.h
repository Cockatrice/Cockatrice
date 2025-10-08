/**
 * @file local_server.h
 * @ingroup Server
 * @brief TODO: Document this.
 */

#ifndef LOCALSERVER_H
#define LOCALSERVER_H

#include <../remote/server.h>
#include <../remote/server_database_interface.h>

class LocalServerInterface;

class LocalServer : public Server
{
    Q_OBJECT
public:
    explicit LocalServer(QObject *parent = nullptr);
    ~LocalServer() override;

    LocalServerInterface *newConnection();
};

class LocalServer_DatabaseInterface : public Server_DatabaseInterface
{
    Q_OBJECT
private:
    LocalServer *localServer;

protected:
    ServerInfo_User getUserData(const QString &name, bool withId = false) override;

public:
    explicit LocalServer_DatabaseInterface(LocalServer *_localServer);
    ~LocalServer_DatabaseInterface() override = default;
    AuthenticationResult checkUserPassword(Server_ProtocolHandler *handler,
                                           const QString &user,
                                           const QString &password,
                                           const QString &clientId,
                                           QString &reasonStr,
                                           int &secondsLeft,
                                           bool passwordNeedsHash) override;
    int getNextGameId() override
    {
        return localServer->getNextLocalGameId();
    }
    int getNextReplayId() override
    {
        return -1;
    }
    int getActiveUserCount(QString /* connectionType */) override
    {
        return 0;
    }
};

#endif
