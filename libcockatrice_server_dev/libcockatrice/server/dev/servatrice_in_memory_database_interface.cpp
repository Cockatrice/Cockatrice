#include "libcockatrice/server/dev/servatrice_in_memory_database_interface.h"

Servatrice_InMemoryDatabaseInterface::Servatrice_InMemoryDatabaseInterface(QObject *parent)
    : Server_DatabaseInterface(parent)
{
}

void Servatrice_InMemoryDatabaseInterface::addUser(const ServerInfo_User &user)
{
    users.insert(QString::fromStdString(user.name()), user);
}

void Servatrice_InMemoryDatabaseInterface::setAuthenticationResult(AuthenticationResult result)
{
    authenticationResult = result;
}

void Servatrice_InMemoryDatabaseInterface::setActiveUserCount(int count)
{
    activeUserCount = count;
}

const QList<Servatrice_InMemoryDatabaseInterface::LogEntry> &Servatrice_InMemoryDatabaseInterface::getLogEntries() const
{
    return logEntries;
}

AuthenticationResult Servatrice_InMemoryDatabaseInterface::checkUserPassword(Server_ProtocolHandler *,
                                                                             const QString &,
                                                                             const QString &,
                                                                             const QString &,
                                                                             QString &,
                                                                             int &,
                                                                             bool)
{
    return authenticationResult;
}

ServerInfo_User Servatrice_InMemoryDatabaseInterface::getUserData(const QString &name, bool withId)
{
    const auto it = users.constFind(name);
    if (it == users.constEnd()) {
        return {};
    }

    ServerInfo_User user = *it;
    if (!withId) {
        user.clear_id();
    }
    return user;
}

int Servatrice_InMemoryDatabaseInterface::getNextGameId()
{
    return nextGameId++;
}

int Servatrice_InMemoryDatabaseInterface::getNextReplayId()
{
    return nextReplayId++;
}

int Servatrice_InMemoryDatabaseInterface::getActiveUserCount(QString)
{
    return activeUserCount;
}

void Servatrice_InMemoryDatabaseInterface::logMessage(const int,
                                                      const QString &senderName,
                                                      const QString &,
                                                      const QString &logMessage,
                                                      LogMessage_TargetType targetType,
                                                      const int targetId,
                                                      const QString &targetName,
                                                      const QString &channel)
{
    logEntries.append(LogEntry{.senderName = senderName,
                               .message = logMessage,
                               .targetType = targetType,
                               .targetId = targetId,
                               .targetName = targetName,
                               .channel = channel});
}
