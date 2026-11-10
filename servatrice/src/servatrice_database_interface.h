#ifndef SERVATRICE_DATABASE_INTERFACE_H
#define SERVATRICE_DATABASE_INTERFACE_H

#include "libcockatrice/protocol/pb/serverinfo_user.pb.h"

#include <QHash>
#include <QList>
#include <QSqlDatabase>
#include <QString>
#include <QStringList>
#include <QtGlobal>
#include <libcockatrice/protocol/pb/serverinfo_ban.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_chat_message.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_moderator_login.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_private_chat_message.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_user_alt.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_user_session.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_warning.pb.h>
#include <qtmetamacros.h>
#include <server.h>
#include <server_database_interface.h>

class QSqlQuery;

#define DATABASE_SCHEMA_VERSION 40

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
    /** Seal a legacy credential for the account's downgrade backup; empty when the
     *  backup key is unset or the sealed value does not fit the column. */
    QString sealLegacyBackup(const QString &storedName, const QString &credential);
    bool isStrictModeEnabled(bool &ok) const;
    int getDeckPathIdAt(int basePathId, QStringList path, int userId);
    void deleteDeckFolderTree(int basePathId);
    bool addToList(const QString &table, int userId, int targetId);
    bool removeFromList(const QString &table, int userId, int targetId);

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
    bool checkSql() override;
    QSqlQuery *prepareQuery(const QString &queryText);
    bool execSqlQuery(QSqlQuery *query);
    const QSqlDatabase &getDatabase()
    {
        return sqlDatabase;
    }

    bool activeUserExists(const QString &user) override;
    bool userExists(const QString &user) override;
    QString getUserSalt(const QString &user) override;
    QString getUserPasswordData(const QString &user) override;
    bool submitPasswordVerifier(const QString &user, const QString &passwordVerifier) override;
    /** @brief Decrypt the account's legacy password backup when it exists, is decryptable
     *  with the configured key and has not aged out; purges it when it has. Returns the
     *  legacy credential in the stored "salt + hash" format, or an empty string. */
    QString getUsableLegacyBackup(const QString &user);
    /** @brief Refresh the age window of the account's legacy password backup. */
    void touchLegacyBackup(const QString &user);
    /** @brief Drop the account's legacy password backup. */
    void clearLegacyBackup(const QString &user);
    /** @brief Drop every legacy password backup that has not been used within the TTL. */
    void purgeExpiredLegacyBackups();
    int getUserIdInDB(const QString &name);
    int getUserId(const QString &name) override;
    bool addToBuddyList(int userId, int targetId) override;
    bool removeFromBuddyList(int userId, int targetId) override;
    bool addToIgnoreList(int userId, int targetId) override;
    bool removeFromIgnoreList(int userId, int targetId) override;
    int getDeckPathId(const QString &path, int userId) override;
    int getDeckOwnerId(int deckId) override;
    bool isDeckEffectivelyPublic(int deckId) override;
    DeckStorageResult setDeckVisibility(int deckId, int userId, bool isPublic) override;
    DeckStorageResult setDeckFolderVisibility(int folderId, int userId, bool isPublic) override;
    DeckStorageResult createDeckFolder(int parentId, int userId, const QString &name) override;
    void deleteDeckFolder(int folderId) override;
    DeckFileWriteResult createDeckFile(int folderId, int userId, const DeckFileData &data) override;
    DeckFileWriteResult
    updateDeckFile(int deckId, int userId, const DeckFileData &data, bool updateVisibility) override;
    DeckStorageResult deleteDeckFile(int deckId, int userId) override;
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

    /**
     * @brief Creates a new temporary deck share bundle.
     * @param expiresAt Receives the actual expiry read back from the database.
     * @return false on failure.
     */
    bool createDeckShare(const QString &token,
                         const QString &name,
                         int userId,
                         const QList<DeckShareItemRecord> &items,
                         int expiryDays,
                         qint64 &expiresAt) override;
    int getDeckSharesCreatedToday(int userId) override;
    /** @brief Lists the share bundles created by a user, newest first. */
    bool getDeckSharesForUser(int userId, QList<DeckShareSummaryRecord> &shares) override;
    /**
     * @brief Deletes one of a user's own share bundles (cascades to its items).
     * @return false if no such bundle belongs to the user.
     */
    bool deleteDeckShare(int shareId, int userId) override;
    /**
     * @brief Looks up a valid (non-expired) share bundle by token.
     * @return false if the token is unknown or expired.
     */
    bool getDeckShareList(const QString &token,
                          QString &name,
                          qint64 &expiresAt,
                          QList<DeckShareItemRecord> &items) override;
    /**
     * @brief Fetches the content of one item of a valid share bundle.
     * @return false if the token is unknown/expired or the item does not belong to the bundle.
     */
    bool getDeckShareItem(const QString &token, int itemId, QString &content) override;
    /** @brief Id and color identity of every deck file in a folder, for the share builder. */
    bool getDeckFilesInFolder(int folderId, int userId, QList<DeckFileListItem> &files) override;
    QList<ReplayMatchRecord> getReplayList(int userId) override;
    ReplayAccessResult getReplayData(int replayId, int userId, QByteArray &data) override;
    ReplayWriteResult setReplayDoNotHide(int gameId, int userId, bool doNotHide) override;
    ReplayWriteResult deleteReplayAccess(int gameId, int userId) override;
    ReplayAccessResult checkReplayAccess(int gameId, int userId) override;
    ReplayAccessResult getReplayName(int gameId, QString &replayName) override;
    bool getReplayHashSource(int gameId, QByteArray &source) override;
    bool addReplayAccess(int gameId, int userId, const QString &replayName) override;
    ReplayAccessResult getLatestReplayForGame(int gameId, int &replayId, QByteArray &data) override;
    ReplayWriteResult grantReplayAccessByName(int gameId, const QString &userName) override;
    /** @brief Deletes all expired share bundles (cascades to their items). */
    void cleanupExpiredDeckShares();

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
                    const QString &targetName,
                    const QString &channel = QString()) override;
    bool changeUserPassword(const QString &user, const QString &password, bool passwordNeedsHash) override;
    /** Replace a credential with an already-derived verifier, keeping a sealed legacy hash of the
     *  same password as the downgrade backup for pre-challenge clients. */
    bool changeUserPasswordWithLegacyBackup(const QString &user,
                                            const QString &passwordVerifier,
                                            const QString &legacyCredential);
    bool changeUserPassword(const QString &user,
                            const QString &oldPassword,
                            bool oldPasswordNeedsHash,
                            const QString &newPassword,
                            bool newPasswordNeedsHash) override;
    void setForcePasswordChange(const QString &user, bool force) override;
    QList<ServerInfo_Ban> getUserBanHistory(const QString &userName) override;
    bool
    addWarning(const QString userName, const QString adminName, const QString warningReason, const QString clientID);
    QList<ServerInfo_Warning> getUserWarnHistory(const QString &userName) override;
    QList<ServerInfo_ChatMessage> getMessageLogHistory(const QString &user,
                                                       const QString &ipaddress,
                                                       const QString &gamename,
                                                       const QString &gameid,
                                                       const QString &message,
                                                       bool &chat,
                                                       bool &game,
                                                       bool &room,
                                                       int &range,
                                                       int &maxresults,
                                                       int offset = 0,
                                                       const QString &beforeTime = QString(),
                                                       bool *hasMore = nullptr) override;
    QList<ServerInfo_UserSession> getUserSessions(const QString &userName, int limit) override;
    bool getPrivateChatHistory(int callerId,
                               const QString &callerName,
                               int peerId,
                               const QString &peerName,
                               quint64 before,
                               int limit,
                               QList<ServerInfo_PrivateChatMessage> &messages,
                               bool &hasMore) override;
    QList<ServerInfo_UserAlt> getUserAlts(const QString &userName,
                                          int depth = 1,
                                          bool useEmail = true,
                                          bool useClientId = true,
                                          bool useIp = true,
                                          int sessionMonths = 6) override;
    QList<ServerInfo_ModeratorLogin> getModeratorLastLogins() override;

    // Uptime snapshot as recorded by Servatrice::statusUpdate() into the
    // {prefix}_uptime table. valid is false when no snapshot exists yet.
    UptimeSnapshot getLatestUptimeSnapshot(int serverId) override;
    bool removeUserAvatar(const QString &userName);

    // Outcome of removeUserBanner(), so the caller can tell a missing user apart
    // from a user who simply has no banner card set.
    enum class RemoveUserBannerResult
    {
        Removed,
        NothingToRemove,
        NoSuchUser,
        DatabaseError,
    };
    RemoveUserBannerResult removeUserBanner(const QString &userName);
    int clearBlacklistedBanners();
    bool isCardArtAllowed(const QString &cardName, const QString &cardProviderId) override;
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
                        bool results) override;
};

#endif
