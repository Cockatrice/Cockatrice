#ifndef SERVER_DATABASE_INTERFACE_H
#define SERVER_DATABASE_INTERFACE_H

#include <QObject>
#include <QSettings>

#include "server.h"

class Server_DatabaseInterface : public QObject {
    Q_OBJECT
public:
    Server_DatabaseInterface(QObject *parent = 0)
        : QObject(parent) { }
    
    virtual AuthenticationResult checkUserPassword(Server_ProtocolHandler *handler, const QString &user, const QString &password, QString &reasonStr, int &secondsLeft) = 0;
    virtual bool checkUserIsBanned(const QString & /* ipAddress */, const QString & /* userName */, QString & /* banReason */, int & /* banSecondsRemaining */) { return false; }
    virtual bool activeUserExists(const QString & /* user */) { return false; }
    virtual bool userExists(const QString & /* user */) { return false; }
    virtual QMap<QString, ServerInfo_User> getBuddyList(const QString & /* name */) { return QMap<QString, ServerInfo_User>(); }
    virtual QMap<QString, ServerInfo_User> getIgnoreList(const QString & /* name */) { return QMap<QString, ServerInfo_User>(); }
    virtual bool isInBuddyList(const QString & /* whoseList */, const QString & /* who */) { return false; }
    virtual bool isInIgnoreList(const QString & /* whoseList */, const QString & /* who */) { return false; }
    virtual ServerInfo_User getUserData(const QString &name, bool withId = false) = 0;
    virtual void storeGameInformation(const QString & /* roomName */, const QStringList & /* roomGameTypes */, const ServerInfo_Game & /* gameInfo */, const QSet<QString> & /* allPlayersEver */, const QSet<QString> & /* allSpectatorsEver */, const QList<GameReplay *> & /* replayList */) { }
    virtual DeckList *getDeckFromDatabase(int /* deckId */, int /* userId */) { return 0; }
    
    virtual qint64 startSession(const QString & /* userName */, const QString & /* address */) { return 0; }
    virtual bool usernameIsValid(const QString & /*userName */, QString & /* error */) { return true; };
public slots:
    virtual void endSession(qint64 /* sessionId */ ) { }
public:
    virtual int getNextGameId() = 0;
    virtual int getNextReplayId() = 0;
    
    virtual void clearSessionTables() { }
    virtual void lockSessionTables() { }
    virtual void unlockSessionTables() { }
    virtual bool userSessionExists(const QString & /* userName */) { return false; }

    virtual bool getRequireRegistration() { return false; }
    virtual bool registerUser(const QString & /* userName */, const QString & /* realName */, ServerInfo_User_Gender const & /* gender */, const QString & /* password */, const QString & /* emailAddress */, const QString & /* country */, bool /* active = false */) { return false; }
    virtual bool activateUser(const QString & /* userName */, const QString & /* token */) { return false; }

    enum LogMessage_TargetType { MessageTargetRoom, MessageTargetGame, MessageTargetChat, MessageTargetIslRoom };
    virtual void logMessage(const int /* senderId */, const QString & /* senderName */, const QString & /* senderIp */, const QString & /* logMessage */, LogMessage_TargetType /* targetType */, const int /* targetId */, const QString & /* targetName */) { };
    bool checkUserIsBanned(Server_ProtocolHandler *session, QString &banReason, int &banSecondsRemaining);
};

#endif
