/** @file server_test_fixture.cpp
 *  @brief Shared in-memory server database double and gtest fixture for server tests.
 *  @ingroup Tests
 */

#include "support/server_test_fixture.h"

#include "server_room.h"

#include <utility>

InMemoryServer_DatabaseInterface::InMemoryServer_DatabaseInterface(QObject *parent) : Server_DatabaseInterface(parent)
{
}

void InMemoryServer_DatabaseInterface::addUser(const ServerInfo_User &user)
{
    users.insert(QString::fromStdString(user.name()), user);
}

AuthenticationResult InMemoryServer_DatabaseInterface::checkUserPassword(Server_ProtocolHandler *,
                                                                         const QString &,
                                                                         const QString &,
                                                                         const QString &,
                                                                         QString &,
                                                                         int &,
                                                                         bool)
{
    return authenticationResult;
}

bool InMemoryServer_DatabaseInterface::checkUserIsBanned(const QString &,
                                                         const QString &,
                                                         const QString &,
                                                         QString &reason,
                                                         int &seconds)
{
    reason = banReason;
    seconds = banSecondsRemaining;
    return userIsBanned;
}

bool InMemoryServer_DatabaseInterface::checkUserIsBanned(Server_ProtocolHandler *, QString &reason, int &seconds)
{
    reason = banReason;
    seconds = banSecondsRemaining;
    return userIsBanned;
}

bool InMemoryServer_DatabaseInterface::activeUserExists(const QString &user)
{
    for (const QString &sessionUser : std::as_const(sessions)) {
        if (sessionUser == user) {
            return true;
        }
    }
    return false;
}

ServerInfo_User InMemoryServer_DatabaseInterface::getUserData(const QString &name, bool withId)
{
    ServerInfo_User user = users.value(name);
    if (!withId) {
        user.clear_id();
    }
    return user;
}

qint64 InMemoryServer_DatabaseInterface::startSession(const QString &userName,
                                                      const QString &,
                                                      const QString &,
                                                      const QString &)
{
    const qint64 sessionId = nextSessionId++;
    sessions.insert(sessionId, userName);
    return sessionId;
}

void InMemoryServer_DatabaseInterface::endSession(qint64 sessionId)
{
    sessions.remove(sessionId);
}

bool InMemoryServer_DatabaseInterface::userSessionExists(const QString &userName)
{
    return activeUserExists(userName);
}

bool InMemoryServer_DatabaseInterface::getRequireRegistration()
{
    return requireRegistration;
}

int InMemoryServer_DatabaseInterface::getNextGameId()
{
    return nextGameId;
}

int InMemoryServer_DatabaseInterface::getNextReplayId()
{
    return nextReplayId;
}

int InMemoryServer_DatabaseInterface::getActiveUserCount(QString)
{
    return activeUserCount;
}

void InMemoryServer_DatabaseInterface::logMessage(const int,
                                                  const QString &senderName,
                                                  const QString &,
                                                  const QString &message,
                                                  LogMessage_TargetType targetType,
                                                  const int targetId,
                                                  const QString &targetName,
                                                  const QString &channel)
{
    logEntries.append(LogEntry{.senderName = senderName,
                               .message = message,
                               .targetType = targetType,
                               .targetId = targetId,
                               .targetName = targetName,
                               .channel = channel});
}

FakeServer::FakeServer()
{
    setDatabaseInterface(new InMemoryServer_DatabaseInterface(this));
}

InMemoryServer_DatabaseInterface *FakeServer::database() const
{
    return static_cast<InMemoryServer_DatabaseInterface *>(getDatabaseInterface());
}

void FakeServer::insertRoom(Server_Room *newRoom)
{
    addRoom(newRoom);
}
