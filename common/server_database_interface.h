#ifndef SERVER_DATABASE_INTERFACE_H
#define SERVER_DATABASE_INTERFACE_H

#include "server.h"

#include <QObject>

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
    };
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
        MessageTargetIslRoom
    };
    virtual void logMessage(const int /* senderId */,
                            const QString & /* senderName */,
                            const QString & /* senderIp */,
                            const QString & /* logMessage */,
                            LogMessage_TargetType /* targetType */,
                            const int /* targetId */,
                            const QString & /* targetName */){};
    virtual bool checkUserIsBanned(Server_ProtocolHandler * /* session */,
                                   QString & /* banReason */,
                                   int & /* banSecondsRemaining */)
    {
        return false;
    };
    virtual int checkNumberOfUserAccounts(const QString & /* email */)
    {
        return 0;
    };
    virtual bool
    changeUserPassword(const QString & /* user */, const QString & /* password */, bool /* passwordNeedsHash */)
    {
        return false;
    };
    virtual bool changeUserPassword(const QString & /* user */,
                                    const QString & /* oldPassword */,
                                    bool /* oldPasswordNeedsHash */,
                                    const QString & /* newPassword */,
                                    bool /* newPasswordNeedsHash */)
    {
        return false;
    };
};

#endif
