/** @file server_test_fixture.h
 *  @brief Shared in-memory server database double and gtest fixture for server tests.
 *  @ingroup Tests
 */

#ifndef COCKATRICE_TESTS_SUPPORT_SERVER_TEST_FIXTURE_H
#define COCKATRICE_TESTS_SUPPORT_SERVER_TEST_FIXTURE_H

#include "server.h"
#include "server_database_interface.h"

#include <QHash>
#include <QList>
#include <QString>
#include <gtest/gtest.h>
#include <libcockatrice/protocol/pb/serverinfo_user.pb.h>

class Server_Room;

/**
 * @brief A Server_DatabaseInterface backed by in-memory state.
 *
 * Only the methods the server tests need are overridden; everything else keeps
 * the abstract interface's defaults. The public members are the knobs a test
 * turns to describe the state it wants.
 */
class InMemoryServer_DatabaseInterface : public Server_DatabaseInterface
{
public:
    explicit InMemoryServer_DatabaseInterface(QObject *parent = nullptr);

    struct LogEntry
    {
        QString senderName;
        QString message;
        LogMessage_TargetType targetType;
        int targetId;
        QString targetName;
        QString channel;
    };

    AuthenticationResult authenticationResult = NotLoggedIn;
    QHash<QString, ServerInfo_User> users;
    bool userIsBanned = false;
    QString banReason;
    int banSecondsRemaining = 0;
    int activeUserCount = 0;
    int nextGameId = 1;
    int nextReplayId = 1;
    bool requireRegistration = false;
    QList<LogEntry> logEntries;

    void addUser(const ServerInfo_User &user);

    AuthenticationResult checkUserPassword(Server_ProtocolHandler *handler,
                                           const QString &user,
                                           const QString &password,
                                           const QString &clientId,
                                           QString &reasonStr,
                                           int &secondsLeft,
                                           bool passwordNeedsHash) override;
    bool checkUserIsBanned(const QString &ipAddress,
                           const QString &userName,
                           const QString &clientId,
                           QString &reason,
                           int &seconds) override;
    bool checkUserIsBanned(Server_ProtocolHandler *session, QString &reason, int &seconds) override;
    bool activeUserExists(const QString &user) override;
    ServerInfo_User getUserData(const QString &name, bool withId = false) override;
    qint64 startSession(const QString &userName,
                        const QString &address,
                        const QString &clientId,
                        const QString &connectionType) override;
    void endSession(qint64 sessionId) override;
    bool userSessionExists(const QString &userName) override;
    bool getRequireRegistration() override;
    int getNextGameId() override;
    int getNextReplayId() override;
    int getActiveUserCount(QString connectionType = QString()) override;
    void logMessage(const int senderId,
                    const QString &senderName,
                    const QString &senderIp,
                    const QString &message,
                    LogMessage_TargetType targetType,
                    const int targetId,
                    const QString &targetName,
                    const QString &channel = QString()) override;

private:
    QHash<qint64, QString> sessions;
    qint64 nextSessionId = 1;
};

/// A Server that always has its own InMemoryServer_DatabaseInterface attached.
class FakeServer : public Server
{
public:
    FakeServer();

    InMemoryServer_DatabaseInterface *database() const;

    void insertRoom(Server_Room *newRoom);
};

/// A gtest fixture with a FakeServer and direct access to its database.
class ServerTestFixture : public ::testing::Test
{
protected:
    FakeServer server;

    InMemoryServer_DatabaseInterface *database() const
    {
        return server.database();
    }
};

#endif
