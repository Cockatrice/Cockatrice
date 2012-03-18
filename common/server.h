#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QStringList>
#include <QMap>
#include <QMultiMap>
#include <QMutex>
#include <QReadWriteLock>
#include "pb/serverinfo_user.pb.h"
#include "server_player_reference.h"

class Server_Game;
class Server_Room;
class Server_ProtocolHandler;
class Server_AbstractUserInterface;
class GameReplay;
class IslMessage;
class SessionEvent;
class RoomEvent;
class DeckList;
class ServerInfo_Game;
class ServerInfo_Room;
class Response;
class GameEventContainer;
class CommandContainer;
class Command_JoinGame;

enum AuthenticationResult { NotLoggedIn = 0, PasswordRight = 1, UnknownUser = 2, WouldOverwriteOldSession = 3, UserIsBanned = 4 };

class Server : public QObject
{
	Q_OBJECT
signals:
	void pingClockTimeout();
	void sigSendIslMessage(const IslMessage &message, int serverId);
	void logDebugMessage(QString message, void *caller);
private slots:
	void broadcastRoomUpdate(const ServerInfo_Room &roomInfo, bool sendToIsl = false);
public:
	mutable QReadWriteLock clientsLock, roomsLock; // locking order: roomsLock before clientsLock
	Server(QObject *parent = 0);
	~Server();
	AuthenticationResult loginUser(Server_ProtocolHandler *session, QString &name, const QString &password, QString &reason);
	const QMap<int, Server_Room *> &getRooms() { return rooms; }
	virtual int getNextGameId() { return nextGameId++; }
	virtual int getNextReplayId() { return nextReplayId++; }
	
	const QMap<QString, Server_ProtocolHandler *> &getUsers() const { return users; }
	const QMap<qint64, Server_ProtocolHandler *> &getUsersBySessionId() const { return usersBySessionId; }
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

	void sendIsl_Response(const Response &item, int serverId = -1, qint64 sessionId = -1);
	void sendIsl_SessionEvent(const SessionEvent &item, int serverId = -1, qint64 sessionId = -1);
	void sendIsl_GameEventContainer(const GameEventContainer &item, int serverId = -1, qint64 sessionId = -1);
	void sendIsl_RoomEvent(const RoomEvent &item, int serverId = -1, qint64 sessionId = -1);
	void sendIsl_GameCommand(const CommandContainer &item, int serverId, qint64 sessionId, int roomId, int playerId);
	void sendIsl_RoomCommand(const CommandContainer &item, int serverId, qint64 sessionId, int roomId);
	
	void addExternalUser(const ServerInfo_User &userInfo);
	void removeExternalUser(const QString &userName);
	const QMap<QString, Server_AbstractUserInterface *> &getExternalUsers() const { return externalUsers; }
	
	void addPersistentPlayer(const QString &userName, int roomId, int gameId, int playerId);
	void removePersistentPlayer(const QString &userName, int roomId, int gameId, int playerId);
	QList<PlayerReference> getPersistentPlayerReferences(const QString &userName) const;
private:
	QMultiMap<QString, PlayerReference> persistentPlayers;
	mutable QReadWriteLock persistentPlayersLock;
protected slots:	
	void externalUserJoined(const ServerInfo_User &userInfo);
	void externalUserLeft(const QString &userName);
	void externalRoomUserJoined(int roomId, const ServerInfo_User &userInfo);
	void externalRoomUserLeft(int roomId, const QString &userName);
	void externalRoomSay(int roomId, const QString &userName, const QString &message);
	void externalRoomGameListChanged(int roomId, const ServerInfo_Game &gameInfo);
	void externalJoinGameCommandReceived(const Command_JoinGame &cmd, int cmdId, int roomId, int serverId, qint64 sessionId);
	void externalGameCommandContainerReceived(const CommandContainer &cont, int playerId, int serverId, qint64 sessionId);
	void externalGameEventContainerReceived(const GameEventContainer &cont, qint64 sessionId);
	void externalResponseReceived(const Response &resp, qint64 sessionId);
	
	virtual void doSendIslMessage(const IslMessage &msg, int serverId) { }
protected:
	void prepareDestroy();
	QList<Server_ProtocolHandler *> clients;
	QMap<qint64, Server_ProtocolHandler *> usersBySessionId;
	QMap<QString, Server_ProtocolHandler *> users;
	QMap<qint64, Server_AbstractUserInterface *> externalUsersBySessionId;
	QMap<QString, Server_AbstractUserInterface *> externalUsers;
	QMap<int, Server_Room *> rooms;
	
	virtual qint64 startSession(const QString &userName, const QString &address) { return -1; }
	virtual void endSession(qint64 sessionId) { }
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
};

#endif
