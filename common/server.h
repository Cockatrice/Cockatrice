#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QStringList>
#include <QMap>
#include <QMutex>
#include <QReadWriteLock>
#include <QMetaType>
#include "pb/serverinfo_user.pb.h"
#include "pb/serverinfo_room.pb.h"
#include "pb/serverinfo_game.pb.h"

class Server_Game;
class Server_Room;
class Server_ProtocolHandler;
class Server_AbstractUserInterface;
class GameReplay;
class IslMessage;
class Response;
class SessionEvent;
class GameEventContainer;
class RoomEvent;
class DeckList;

enum AuthenticationResult { NotLoggedIn = 0, PasswordRight = 1, UnknownUser = 2, WouldOverwriteOldSession = 3, UserIsBanned = 4 };

class Server : public QObject
{
	Q_OBJECT
signals:
	void pingClockTimeout();
private slots:
	void broadcastRoomUpdate(const ServerInfo_Room &roomInfo, bool sendToIsl = false);
public:
	mutable QReadWriteLock clientsLock, roomsLock;
	Server(QObject *parent = 0);
	~Server();
	AuthenticationResult loginUser(Server_ProtocolHandler *session, QString &name, const QString &password, QString &reason);
	const QMap<int, Server_Room *> &getRooms() { return rooms; }
	virtual int getNextGameId() { return nextGameId++; }
	virtual int getNextReplayId() { return nextReplayId++; }
	
	const QMap<QString, Server_ProtocolHandler *> &getUsers() const { return users; }
	void addClient(Server_ProtocolHandler *player);
	void removeClient(Server_ProtocolHandler *player);
	virtual QString getLoginMessage() const { return QString(); }
	
	virtual bool getGameShouldPing() const { return false; }
	virtual int getMaxGameInactivityTime() const { return 9999999; }
	virtual int getMaxPlayerInactivityTime() const { return 9999999; }
	virtual int getMessageCountingInterval() const { return 0; }
	virtual int getMaxMessageCountPerInterval() const { return 0; }
	virtual int getMaxMessageSizePerInterval() const { return 0; }
	virtual int getMaxGamesPerUser() const { return 0; }
	virtual bool getThreaded() const { return false; }
	
	virtual QMap<QString, ServerInfo_User> getBuddyList(const QString &name) { return QMap<QString, ServerInfo_User>(); }
	virtual QMap<QString, ServerInfo_User> getIgnoreList(const QString &name) { return QMap<QString, ServerInfo_User>(); }
	virtual bool isInBuddyList(const QString &whoseList, const QString &who) { return false; }
	virtual bool isInIgnoreList(const QString &whoseList, const QString &who) { return false; }
	
	virtual void storeGameInformation(int secondsElapsed, const QSet<QString> &allPlayersEver, const QSet<QString> &allSpectatorsEver, const QList<GameReplay *> &replays) { }
	virtual DeckList *getDeckFromDatabase(int deckId, const QString &userName) { return 0; }

	void sendIslMessage(const Response &item, int serverId = -1);
	void sendIslMessage(const SessionEvent &item, int serverId = -1);
	void sendIslMessage(const GameEventContainer &item, int serverId = -1);
	void sendIslMessage(const RoomEvent &item, int serverId = -1);
	
	void addExternalUser(const ServerInfo_User &userInfo);
	void removeExternalUser(const QString &userName);
	const QMap<QString, Server_AbstractUserInterface *> &getExternalUsers() const { return externalUsers; }
protected slots:	
	void externalUserJoined(const ServerInfo_User &userInfo);
	void externalUserLeft(const QString &userName);
	void externalRoomUserJoined(int roomId, const ServerInfo_User &userInfo);
	void externalRoomUserLeft(int roomId, const QString &userName);
	void externalRoomSay(int roomId, const QString &userName, const QString &message);
	void externalRoomGameListChanged(int roomId, const ServerInfo_Game &gameInfo);
protected:
	void prepareDestroy();
	QList<Server_ProtocolHandler *> clients;
	QMap<QString, Server_ProtocolHandler *> users;
	QMap<QString, Server_AbstractUserInterface *> externalUsers;
	QMap<int, Server_Room *> rooms;
	
	virtual int startSession(const QString &userName, const QString &address) { return -1; }
	virtual void endSession(int sessionId) { }
	virtual bool userExists(const QString &user) { return false; }
	virtual AuthenticationResult checkUserPassword(Server_ProtocolHandler *handler, const QString &user, const QString &password, QString &reason) { return UnknownUser; }
	virtual ServerInfo_User getUserData(const QString &name, bool withId = false) = 0;
	int getUsersCount() const;
	int getGamesCount() const;
	int nextGameId, nextReplayId;
	void addRoom(Server_Room *newRoom);

	virtual void clearSessionTables() { }
	virtual void lockSessionTables() { }
	virtual void unlockSessionTables() { }
	virtual bool userSessionExists(const QString &userName) { return false; }
	
	virtual void doSendIslMessage(const IslMessage &msg, int serverId) { }
};

Q_DECLARE_METATYPE(ServerInfo_User)
Q_DECLARE_METATYPE(ServerInfo_Room)
Q_DECLARE_METATYPE(ServerInfo_Game)

#endif
