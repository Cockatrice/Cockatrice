#ifndef SERVATRICE_IN_MEMORY_DATABASE_INTERFACE_H
#define SERVATRICE_IN_MEMORY_DATABASE_INTERFACE_H

#include "server_database_interface.h"

#include <QHash>
#include <QList>
#include <QString>

/**
 * @brief In-memory Server_DatabaseInterface for tests and the dev server.
 *
 * Holds the little state a server needs to come up without a SQL database:
 * registered users, the authentication outcome, the logged messages, the active
 * user count and the game/replay id allocators. Calls that need persistence keep
 * the interface's fail-closed defaults.
 */
class Servatrice_InMemoryDatabaseInterface : public Server_DatabaseInterface
{
public:
    /** @brief One recorded logMessage() call. */
    struct LogEntry
    {
        QString senderName;
        QString message;
        LogMessage_TargetType targetType;
        int targetId;
        QString targetName;
        QString channel;
    };

    explicit Servatrice_InMemoryDatabaseInterface(QObject *parent = nullptr);

    /** @brief Registers the user getUserData() returns; the id is only exposed to withId lookups. */
    void addUser(const ServerInfo_User &user);
    /** @brief Sets the result every checkUserPassword() call returns. */
    void setAuthenticationResult(AuthenticationResult result);
    /** @brief Sets the value getActiveUserCount() reports. */
    void setActiveUserCount(int count);

    const QList<LogEntry> &getLogEntries() const;

    AuthenticationResult checkUserPassword(Server_ProtocolHandler * /* handler */,
                                           const QString & /* user */,
                                           const QString & /* password */,
                                           const QString & /* clientId */,
                                           QString & /* reasonStr */,
                                           int & /* secondsLeft */,
                                           bool /* passwordNeedsHash */) override;
    ServerInfo_User getUserData(const QString &name, bool withId = false) override;
    int getNextGameId() override;
    int getNextReplayId() override;
    int getActiveUserCount(QString connectionType = QString()) override;
    void logMessage(const int /* senderId */,
                    const QString &senderName,
                    const QString & /* senderIp */,
                    const QString &logMessage,
                    LogMessage_TargetType targetType,
                    const int targetId,
                    const QString &targetName,
                    const QString &channel = QString()) override;

private:
    QHash<QString, ServerInfo_User> users;
    AuthenticationResult authenticationResult = NotLoggedIn;
    int activeUserCount = 0;
    int nextGameId = 1;
    int nextReplayId = 1;
    QList<LogEntry> logEntries;
};

#endif // SERVATRICE_IN_MEMORY_DATABASE_INTERFACE_H
