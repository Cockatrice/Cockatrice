#ifndef SERVER_DATABASE_INTERFACE_H
#define SERVER_DATABASE_INTERFACE_H

#include "libcockatrice/protocol/pb/serverinfo_ban.pb.h"
#include "libcockatrice/protocol/pb/serverinfo_chat_message.pb.h"
#include "libcockatrice/protocol/pb/serverinfo_moderator_login.pb.h"
#include "libcockatrice/protocol/pb/serverinfo_private_chat_message.pb.h"
#include "libcockatrice/protocol/pb/serverinfo_user.pb.h"
#include "libcockatrice/protocol/pb/serverinfo_user_alt.pb.h"
#include "libcockatrice/protocol/pb/serverinfo_user_session.pb.h"
#include "libcockatrice/protocol/pb/serverinfo_warning.pb.h"
#include "server.h"

#include <QByteArray>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QtGlobal>
#include <qtmetamacros.h>

class DeckList;
class GameReplay;
class ServerInfo_Game;
class Server_ProtocolHandler;
template <typename T> class QList;
template <typename T> class QSet;

class Server_DatabaseInterface : public QObject
{
    Q_OBJECT
public:
    explicit Server_DatabaseInterface(QObject *parent = nullptr) : QObject(parent)
    {
    }

    virtual AuthenticationResult checkUserPassword(Server_ProtocolHandler *handler,
                                                   const QString &user,
                                                   const QString &password,
                                                   const QString &clientId,
                                                   QString &reasonStr,
                                                   int &secondsLeft,
                                                   bool passwordNeedsHash) = 0;
    virtual bool checkUserIsBanned(const QString & /* ipAddress */,
                                   const QString & /* userName */,
                                   const QString & /* clientId */,
                                   QString & /* banReason */,
                                   int & /* banSecondsRemaining */)
    {
        return false;
    }
    virtual bool activeUserExists(const QString & /* user */)
    {
        return false;
    }
    virtual bool userExists(const QString & /* user */)
    {
        return false;
    }
    virtual QString getUserSalt(const QString & /* user */)
    {
        return {};
    }
    virtual QString getUserPasswordData(const QString & /* user */)
    {
        return {};
    }
    virtual bool submitPasswordVerifier(const QString & /* user */, const QString & /* passwordVerifier */)
    {
        return false;
    }
    virtual QMap<QString, ServerInfo_User> getBuddyList(const QString & /* name */)
    {
        return QMap<QString, ServerInfo_User>();
    }
    virtual QMap<QString, ServerInfo_User> getIgnoreList(const QString & /* name */)
    {
        return QMap<QString, ServerInfo_User>();
    }
    virtual bool isInBuddyList(const QString & /* whoseList */, const QString & /* who */)
    {
        return false;
    }
    virtual bool isInIgnoreList(const QString & /* whoseList */, const QString & /* who */)
    {
        return false;
    }
    /** @brief Numeric id of an existing account, or a negative value when absent. */
    virtual int getUserId(const QString & /* userName */)
    {
        return -1;
    }
    virtual bool addToBuddyList(int /* userId */, int /* targetId */)
    {
        return false;
    }
    virtual bool removeFromBuddyList(int /* userId */, int /* targetId */)
    {
        return false;
    }
    virtual bool addToIgnoreList(int /* userId */, int /* targetId */)
    {
        return false;
    }
    virtual bool removeFromIgnoreList(int /* userId */, int /* targetId */)
    {
        return false;
    }
    virtual QList<ServerInfo_Ban> getUserBanHistory(const QString & /* userName */)
    {
        return {};
    }
    virtual QList<ServerInfo_Warning> getUserWarnHistory(const QString & /* userName */)
    {
        return {};
    }
    virtual QList<ServerInfo_UserSession> getUserSessions(const QString & /* userName */, int /* limit */)
    {
        return {};
    }
    virtual QList<ServerInfo_UserAlt> getUserAlts(const QString & /* userName */,
                                                  int /* depth */ = 1,
                                                  bool /* useEmail */ = true,
                                                  bool /* useClientId */ = true,
                                                  bool /* useIp */ = true,
                                                  int /* sessionMonths */ = 6)
    {
        return {};
    }
    virtual QList<ServerInfo_ModeratorLogin> getModeratorLastLogins()
    {
        return {};
    }
    virtual QList<ServerInfo_ChatMessage> getMessageLogHistory(const QString & /* user */,
                                                               const QString & /* ipaddress */,
                                                               const QString & /* gamename */,
                                                               const QString & /* gameid */,
                                                               const QString & /* message */,
                                                               bool & /* chat */,
                                                               bool & /* game */,
                                                               bool & /* room */,
                                                               int & /* range */,
                                                               int & /* maxresults */,
                                                               int /* offset */ = 0,
                                                               const QString & /* beforeTime */ = QString(),
                                                               bool * /* hasMore */ = nullptr)
    {
        return {};
    }
    virtual bool getPrivateChatHistory(int /* callerId */,
                                       const QString & /* callerName */,
                                       int /* peerId */,
                                       const QString & /* peerName */,
                                       quint64 /* before */,
                                       int /* limit */,
                                       QList<ServerInfo_PrivateChatMessage> & /* messages */,
                                       bool & /* hasMore */)
    {
        return false;
    }
    /**
     * @brief Whether the backing store is reachable, reconnecting when it is not.
     *
     * Implementations that cannot report readiness are always ready, so callers
     * do not have to know which kind of store they hold.
     */
    virtual bool checkSql()
    {
        return true;
    }
    virtual ServerInfo_User getUserData(const QString &name, bool withId = false) = 0;
    virtual void storeGameInformation(const QString & /* roomName */,
                                      const QStringList & /* roomGameTypes */,
                                      const ServerInfo_Game & /* gameInfo */,
                                      const QSet<QString> & /* allPlayersEver */,
                                      const QSet<QString> & /* allSpectatorsEver */,
                                      const QList<GameReplay *> & /* replayList */)
    {
    }
    virtual DeckList *getDeckFromDatabase(int /* deckId */, int /* userId */)
    {
        return 0;
    }
    /** @brief Id of the folder at a "/"-separated path for one user, 0 for the root, -1 when absent. */
    virtual int getDeckPathId(const QString & /* path */, int /* userId */)
    {
        return -1;
    }
    /** @brief Owner of a deck file, or -1 when the deck does not exist. */
    virtual int getDeckOwnerId(int /* deckId */)
    {
        return -1;
    }
    /** @brief Whether a deck is public itself or sits in a public folder. */
    virtual bool isDeckEffectivelyPublic(int /* deckId */)
    {
        return false;
    }
    /** @brief Outcome of a deck-storage write, so callers can map it to a response code. */
    enum class DeckStorageResult
    {
        Ok,
        NotFound,
        Failed
    };
    virtual DeckStorageResult setDeckVisibility(int /* deckId */, int /* userId */, bool /* isPublic */)
    {
        return DeckStorageResult::Failed;
    }
    virtual DeckStorageResult setDeckFolderVisibility(int /* folderId */, int /* userId */, bool /* isPublic */)
    {
        return DeckStorageResult::Failed;
    }
    virtual DeckStorageResult createDeckFolder(int /* parentId */, int /* userId */, const QString & /* name */)
    {
        return DeckStorageResult::Failed;
    }
    /** @brief Delete a folder, its deck files, and every nested folder. */
    virtual void deleteDeckFolder(int /* folderId */)
    {
    }
    /** @brief The mutable deck-file columns, as the upload command supplies them. */
    struct DeckFileData
    {
        QString name;
        QString content;
        bool isPublic = false;
        QString bannerCardName;
        QString bannerCardProvider;
        QString colorIdentity;
        QString tagsJson;
    };
    struct DeckFileWriteResult
    {
        DeckStorageResult result = DeckStorageResult::Failed;
        int deckId = 0;
        bool isPublic = false;
    };
    virtual DeckFileWriteResult createDeckFile(int /* folderId */, int /* userId */, const DeckFileData & /* data */)
    {
        return DeckFileWriteResult{};
    }
    virtual DeckFileWriteResult
    updateDeckFile(int /* deckId */, int /* userId */, const DeckFileData & /* data */, bool /* updateVisibility */)
    {
        return DeckFileWriteResult{};
    }
    virtual DeckStorageResult deleteDeckFile(int /* deckId */, int /* userId */)
    {
        return DeckStorageResult::Failed;
    }
    /** @brief Metadata of a single deck inside a temporary deck share bundle. */
    struct DeckShareItemRecord
    {
        int id = -1;           ///< Database id, used for downloads.
        QString name;          ///< Deck name.
        QStringList tags;      ///< Deck tags.
        QString bannerCard;    ///< Banner card name (deck image).
        QString gameFormat;    ///< Game format the deck was built for.
        QString colorIdentity; ///< Color identity, e.g. "WUBRG".
        QString content;       ///< Deck content (native format); empty in list queries.
    };
    /** @brief Summary of a share bundle owned by a user. */
    struct DeckShareSummaryRecord
    {
        int id = -1;             ///< Database id, used for revocation.
        QString name;            ///< Share name.
        qint64 creationTime = 0; ///< Unix timestamp at which the share was created.
        qint64 expiresAt = 0;    ///< Unix timestamp at which the share expires.
        int itemCount = 0;       ///< Number of decks in the bundle.
    };
    /** @brief One deck file in a folder, as the share builder lists it. */
    struct DeckFileListItem
    {
        int id = -1;
        QString colorIdentity;
    };
    virtual bool createDeckShare(const QString & /* token */,
                                 const QString & /* name */,
                                 int /* userId */,
                                 const QList<DeckShareItemRecord> & /* items */,
                                 int /* expiryDays */,
                                 qint64 & /* expiresAt */)
    {
        return false;
    }
    /** @brief Share bundles the user created in the last day, or -1 when the read failed. */
    virtual int getDeckSharesCreatedToday(int /* userId */)
    {
        return -1;
    }
    virtual bool getDeckSharesForUser(int /* userId */, QList<DeckShareSummaryRecord> & /* shares */)
    {
        return false;
    }
    virtual bool deleteDeckShare(int /* shareId */, int /* userId */)
    {
        return false;
    }
    virtual bool getDeckShareList(const QString & /* token */,
                                  QString & /* name */,
                                  qint64 & /* expiresAt */,
                                  QList<DeckShareItemRecord> & /* items */)
    {
        return false;
    }
    virtual bool getDeckShareItem(const QString & /* token */, int /* itemId */, QString & /* content */)
    {
        return false;
    }
    virtual bool getDeckFilesInFolder(int /* folderId */, int /* userId */, QList<DeckFileListItem> & /* files */)
    {
        return false;
    }
    /** @brief One downloaded replay file of a match. */
    struct ReplayEntry
    {
        int replayId = 0;
        int duration = 0;
    };
    /** @brief One replay match visible to a user, with its players and replay files. */
    struct ReplayMatchRecord
    {
        int gameId = 0;
        QString replayName;
        QString roomName;
        int timeStarted = 0;
        int timeFinished = 0;
        QString description;
        bool doNotHide = false;
        QStringList playerNames;
        QList<ReplayEntry> replays;
    };
    /** @brief Why a replay read succeeded or not, so callers can map it to a response code. */
    enum class ReplayAccessResult
    {
        Ok,
        AccessDenied,
        NotFound,
        Failed
    };
    enum class ReplayWriteResult
    {
        Ok,
        NotFound,
        Failed
    };
    virtual QList<ReplayMatchRecord> getReplayList(int /* userId */)
    {
        return {};
    }
    virtual ReplayAccessResult getReplayData(int /* replayId */, int /* userId */, QByteArray & /* data */)
    {
        return ReplayAccessResult::Failed;
    }
    virtual ReplayWriteResult setReplayDoNotHide(int /* gameId */, int /* userId */, bool /* doNotHide */)
    {
        return ReplayWriteResult::Failed;
    }
    virtual ReplayWriteResult deleteReplayAccess(int /* gameId */, int /* userId */)
    {
        return ReplayWriteResult::Failed;
    }
    /** @brief Append a moderator/admin action to the server audit log. */
    virtual void addAuditRecord(const QString & /* user */,
                                const QString & /* ipaddress */,
                                const QString & /* clientid */,
                                const QString & /* action */,
                                const QString & /* details */,
                                bool /* results */)
    {
    }
    virtual bool removeForgotPassword(const QString & /* user */)
    {
        return false;
    }
    virtual qint64 startSession(const QString & /* userName */,
                                const QString & /* address */,
                                const QString & /* clientId */,
                                const QString & /* connectionType */)
    {
        return 0;
    }
    virtual bool usernameIsValid(const QString & /*userName */, QString & /* error */)
    {
        return true;
    }
public slots:
    virtual void endSession(qint64 /* sessionId */)
    {
    }

public:
    virtual int getNextGameId() = 0;
    virtual int getNextReplayId() = 0;
    virtual int getActiveUserCount(QString connectionType = QString()) = 0;

    virtual void clearSessionTables()
    {
    }
    virtual void lockSessionTables()
    {
    }
    virtual void unlockSessionTables()
    {
    }
    virtual bool userSessionExists(const QString & /* userName */)
    {
        return false;
    }

    virtual bool getRequireRegistration()
    {
        return false;
    }
    virtual bool registerUser(const QString & /* userName */,
                              const QString & /* realName */,
                              const QString & /* password */,
                              bool /* passwordNeedsHash */,
                              const QString & /* emailAddress */,
                              const QString & /* country */,
                              bool /* active = false */)
    {
        return false;
    }
    virtual bool activateUser(const QString & /* userName */, const QString & /* token */)
    {
        return false;
    }
    virtual void updateUsersClientID(const QString & /* userName */, const QString & /* userClientID */)
    {
    }
    virtual void updateUsersLastLoginData(const QString & /* userName */, const QString & /* clientVersion */)
    {
    }

    enum LogMessage_TargetType
    {
        MessageTargetRoom,
        MessageTargetGame,
        MessageTargetChat,
        MessageTargetIslRoom,
        MessageTargetRoomChannel
    };
    virtual void logMessage(const int /* senderId */,
                            const QString & /* senderName */,
                            const QString & /* senderIp */,
                            const QString & /* logMessage */,
                            LogMessage_TargetType /* targetType */,
                            const int /* targetId */,
                            const QString & /* targetName */,
                            const QString & /* channel */ = QString())
    {
    }
    virtual bool checkUserIsBanned(Server_ProtocolHandler * /* session */,
                                   QString & /* banReason */,
                                   int & /* banSecondsRemaining */)
    {
        return false;
    }
    virtual int checkNumberOfUserAccounts(const QString & /* email */)
    {
        return 0;
    }
    virtual bool
    changeUserPassword(const QString & /* user */, const QString & /* password */, bool /* passwordNeedsHash */)
    {
        return false;
    }
    virtual bool changeUserPassword(const QString & /* user */,
                                    const QString & /* oldPassword */,
                                    bool /* oldPasswordNeedsHash */,
                                    const QString & /* newPassword */,
                                    bool /* newPasswordNeedsHash */)
    {
        return false;
    }
    virtual void setForcePasswordChange(const QString & /* user */, bool /* force */)
    {
    }
    /**
     * @brief Whether the given card art may be shown (card art rules).
     *
     * Default allows everything; the real server overrides this with a lookup
     * in {prefix}_card_art_name_rules. Empty names are always allowed (clear path).
     */
    virtual bool isCardArtAllowed(const QString & /* cardName */, const QString & /* providerId */)
    {
        return true;
    }
};

#endif
