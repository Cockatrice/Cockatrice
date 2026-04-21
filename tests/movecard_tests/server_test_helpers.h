#include "server.h"
#include "server_database_interface.h"

class MockDatabaseInterface : public Server_DatabaseInterface
{
public:
    AuthenticationResult checkUserPassword(Server_ProtocolHandler *,
                                           const QString &,
                                           const QString &,
                                           const QString &,
                                           QString &,
                                           int &,
                                           bool) override
    {
        return NotLoggedIn;
    }
    ServerInfo_User getUserData(const QString &, bool) override
    {
        return ServerInfo_User();
    }
    int getNextGameId() override
    {
        return 1;
    }
    int getNextReplayId() override
    {
        return 1;
    }
    int getActiveUserCount(QString) override
    {
        return 1;
    }
};

class FakeServer : public Server
{
public:
    FakeServer()
    {
        setDatabaseInterface(new MockDatabaseInterface());
    }
};
