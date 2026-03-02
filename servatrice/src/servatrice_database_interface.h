#ifndef SERVATRICE_DATABASE_INTERFACE_H
#define SERVATRICE_DATABASE_INTERFACE_H

#include <QChar>
#include <QHash>
#include <QObject>
#include <QSqlDatabase>
#include <libcockatrice/protocol/pb/serverinfo_chat_message.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_warning.pb.h>
#include <server.h>
#include <server_database_interface.h>

#define DATABASE_SCHEMA_VERSION 34

class Servatrice;

class Servatrice_DatabaseInterface : public Server_DatabaseInterface
{
    Q_OBJECT
private:
    int instanceId;
    QSqlDatabase sqlDatabase;
    QHash<QString, QSqlQuery *> preparedStatements;
    Servatrice *server;
    ServerInfo_User evalUserQueryResult(const QSqlQuery *query, bool complete, bool withId = false);
    /** Must be called after checkSql and server is known to be in auth mode. */
    bool checkUserIsIdBanned(const QString &clientId, QString &banReason, int &banSecondsRemaining);
    /** Must be called after checkSql and server is known to be in auth mode. */
    bool checkUserIsIpBanned(const QString &ipAddress, QString &banReason, int &banSecondsRemaining);
    /** Must be called after checkSql and server is known to be in auth mode. */
    bool checkUserIsNameBanned(QString const &userName, QString &banReason, int &banSecondsRemaining);

protected:
    AuthenticationResult checkUserPassword(Server_ProtocolHandler *handler,
                                           const QString &user,
                                           const QString &password,
                                           const QString &clientId,
                                           QString &reasonStr,
                                           int &banSecondsLeft,
                                           bool passwordNeedsHash) override;

public slots:
    void initDatabase(const QSqlDatabase &_sqlDatabase);

public:
    explicit Servatrice_DatabaseInterface(int _instanceId, Servatrice *_server);
    ~Servatrice_DatabaseInterface() override;
    bool initDatabase(const QString &type,
                      const QString &hostName,
                      const QString &databaseName,
                      const QString &userName,
                      const QString &password);
    bool openDatabase();
    bool checkSql();
    QSqlQuery *prepareQuery(const QString &queryText);
    bool execSqlQuery(QSqlQuery *query);
    const QSqlDatabase &getDatabase()
    {
        return sqlDatabase;
    }

    bool activeUserExists(const QString &user) override;
    bool userExists(const QString &user) override;
    QString getUserSalt(const QString &user) override;
    int getUserIdInDB(const QString &name);
    QMap<QString, ServerInfo_User> getBuddyList(const QString &name) override;
    QMap<QString, ServerInfo_User> getIgnoreList(const QString &name) override;
    bool isInBuddyList(const QString &whoseList, const QString &who) override;
    bool isInIgnoreList(const QString &whoseList, const QString &who) override;
    ServerInfo_User getUserData(const QString &name, bool withId = false) override;
    void storeGameInformation(const QString &roomName,
                              const QStringList &roomGameTypes,
                              const ServerInfo_Game &gameInfo,
                              const QSet<QString> &allPlayersEver,
                              const QSet<QString> &allSpectatorsEver,
                              const QList<GameReplay *> &replayList) override;
    DeckList *getDeckFromDatabase(int deckId, int userId) override;

    int getNextGameId() override;
    int getNextReplayId() override;
    int getActiveUserCount(QString connectionType = QString()) override;

    qint64 startSession(const QString &userName,
                        const QString &address,
                        const QString &clientId,
                        const QString &connectionType) override;
    void endSession(qint64 sessionId) override;
    void clearSessionTables() override;
    void lockSessionTables() override;
    void unlockSessionTables() override;
    bool userSessionExists(const QString &userName) override;
    bool usernameIsValid(const QString &user, QString &error) override;
    bool checkUserIsBanned(const QString &ipAddress,
                           const QString &userName,
                           const QString &clientId,
                           QString &banReason,
                           int &banSecondsRemaining) override;
    int checkNumberOfUserAccounts(const QString &email) override;
    bool registerUser(const QString &userName,
                      const QString &realName,
                      const QString &password,
                      bool passwordNeedsHash,
                      const QString &emailAddress,
                      const QString &country,
                      bool active = false) override;
    bool activateUser(const QString &userName, const QString &token) override;
    void updateUsersClientID(const QString &userName, const QString &userClientID) override;
    void updateUsersLastLoginData(const QString &userName, const QString &clientVersion) override;
    void logMessage(const int senderId,
                    const QString &senderName,
                    const QString &senderIp,
                    const QString &logMessage,
                    LogMessage_TargetType targetType,
                    const int targetId,
                    const QString &targetName) override;
    bool changeUserPassword(const QString &user, const QString &password, bool passwordNeedsHash) override;
    bool changeUserPassword(const QString &user,
                            const QString &oldPassword,
                            bool oldPasswordNeedsHash,
                            const QString &newPassword,
                            bool newPasswordNeedsHash) override;
    QList<ServerInfo_Ban> getUserBanHistory(const QString userName);
    bool
    addWarning(const QString userName, const QString adminName, const QString warningReason, const QString clientID);
    QList<ServerInfo_Warning> getUserWarnHistory(const QString userName);
    QList<ServerInfo_ChatMessage> getMessageLogHistory(const QString &user,
                                                       const QString &ipaddress,
                                                       const QString &gamename,
                                                       const QString &gameid,
                                                       const QString &message,
                                                       bool &chat,
                                                       bool &game,
                                                       bool &room,
                                                       int &range,
                                                       int &maxresults);
    bool addForgotPassword(const QString &user);
    bool removeForgotPassword(const QString &user) override;
    bool doesForgotPasswordExist(const QString &user);
    bool updateUserToken(const QString &token, const QString &user);
    bool validateTableColumnStringData(const QString &table,
                                       const QString &column,
                                       const QString &_user,
                                       const QString &_datatocheck);
    void addAuditRecord(const QString &user,
                        const QString &ipaddress,
                        const QString &clientid,
                        const QString &action,
                        const QString &details,
                        const bool &results);
};

#endif
