#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QStringList>
#include <QMap>
#include <QMutex>

class Server_Game;
class Server_Room;
class Server_ProtocolHandler;
class ServerInfo_User;

enum AuthenticationResult { PasswordWrong = 0, PasswordRight = 1, UnknownUser = 2, WouldOverwriteOldSession = 3 };

class Server : public QObject
{
	Q_OBJECT
signals:
	void pingClockTimeout();
private slots:
	void broadcastRoomUpdate();
public:
	mutable QMutex serverMutex;
	Server(QObject *parent = 0);
	~Server();
	AuthenticationResult loginUser(Server_ProtocolHandler *session, QString &name, const QString &password);
	const QMap<int, Server_Room *> &getRooms() { return rooms; }
	int getNextGameId() { return nextGameId++; }
	
	const QMap<QString, Server_ProtocolHandler *> &getUsers() const { return users; }
	void addClient(Server_ProtocolHandler *player);
	void removeClient(Server_ProtocolHandler *player);
	virtual QString getLoginMessage() const = 0;
	
	virtual bool getGameShouldPing() const = 0;
	virtual int getMaxGameInactivityTime() const = 0;
	virtual int getMaxPlayerInactivityTime() const = 0;
	virtual int getMessageCountingInterval() const { return 0; }
	virtual int getMaxMessageCountPerInterval() const { return 0; }
	virtual int getMaxMessageSizePerInterval() const { return 0; }
	virtual int getMaxGamesPerUser() const { return 0; }
	
	virtual QMap<QString, ServerInfo_User *> getBuddyList(const QString &name) = 0;
	virtual QMap<QString, ServerInfo_User *> getIgnoreList(const QString &name) = 0;
	virtual bool getUserBanned(Server_ProtocolHandler * /*client*/, const QString & /*userName*/) const { return false; }
protected:
	QList<Server_ProtocolHandler *> clients;
	QMap<QString, Server_ProtocolHandler *> users;
	QMap<int, Server_Room *> rooms;
	
	virtual bool userExists(const QString &user) = 0;
	virtual AuthenticationResult checkUserPassword(const QString &user, const QString &password) = 0;
	virtual ServerInfo_User *getUserData(const QString &name) = 0;
	int getUsersCount() const;
	int getGamesCount() const;
	int nextGameId;
	void addRoom(Server_Room *newRoom);
};

#endif
