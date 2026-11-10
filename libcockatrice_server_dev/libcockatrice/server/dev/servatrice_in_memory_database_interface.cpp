#include "libcockatrice/server/dev/servatrice_in_memory_database_interface.h"

Servatrice_InMemoryDatabaseInterface::Servatrice_InMemoryDatabaseInterface(QObject *parent)
    : Server_DatabaseInterface(parent), state(std::make_shared<State>())
{
}

void Servatrice_InMemoryDatabaseInterface::addUser(const ServerInfo_User &user)
{
    state->users.insert(QString::fromStdString(user.name()), user);
}

void Servatrice_InMemoryDatabaseInterface::setAuthenticationResult(AuthenticationResult result)
{
    state->authenticationResult = result;
}

void Servatrice_InMemoryDatabaseInterface::setActiveUserCount(int count)
{
    state->activeUserCount = count;
}

void Servatrice_InMemoryDatabaseInterface::addRoom(const BootstrapRoomRecord &room)
{
    state->rooms.append(room);
}

QList<Servatrice_InMemoryDatabaseInterface::LogEntry> Servatrice_InMemoryDatabaseInterface::getLogEntries() const
{
    QMutexLocker locker(&state->mutex);
    return state->logEntries;
}

AuthenticationResult Servatrice_InMemoryDatabaseInterface::checkUserPassword(Server_ProtocolHandler *,
                                                                             const QString &,
                                                                             const QString &,
                                                                             const QString &,
                                                                             QString &,
                                                                             int &,
                                                                             bool)
{
    return state->authenticationResult;
}

ServerInfo_User Servatrice_InMemoryDatabaseInterface::getUserData(const QString &name, bool withId)
{
    const auto it = state->users.constFind(name);
    if (it == state->users.constEnd()) {
        // The MySQL implementation always names the requested user, with the base user level.
        ServerInfo_User user;
        user.set_name(name.toStdString());
        user.set_user_level(ServerInfo_User::IsUser);
        return user;
    }

    ServerInfo_User user = *it;
    if (!withId) {
        user.clear_id();
    }
    return user;
}

int Servatrice_InMemoryDatabaseInterface::getNextGameId()
{
    QMutexLocker locker(&state->mutex);
    return state->nextGameId++;
}

int Servatrice_InMemoryDatabaseInterface::getNextReplayId()
{
    QMutexLocker locker(&state->mutex);
    return state->nextReplayId++;
}

int Servatrice_InMemoryDatabaseInterface::getActiveUserCount(QString)
{
    return state->activeUserCount;
}

QList<Server_DatabaseInterface::BootstrapRoomRecord> Servatrice_InMemoryDatabaseInterface::getBootstrapRooms(int)
{
    return state->rooms;
}

Server_DatabaseInterface *Servatrice_InMemoryDatabaseInterface::createWorkerInterface(int)
{
    auto *worker = new Servatrice_InMemoryDatabaseInterface();
    worker->state = state;
    return worker;
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
    QMutexLocker locker(&state->mutex);
    state->logEntries.append(LogEntry{.senderName = senderName,
                                      .message = logMessage,
                                      .targetType = targetType,
                                      .targetId = targetId,
                                      .targetName = targetName,
                                      .channel = channel});
}
