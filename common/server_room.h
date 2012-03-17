#ifndef SERVER_ROOM_H
#define SERVER_ROOM_H

#include <QList>
#include <QMap>
#include <QObject>
#include <QStringList>
#include <QMutex>
#include <QReadWriteLock>
#include "pb/serverinfo_room.pb.h"
#include "serverinfo_user_container.h"

class Server_ProtocolHandler;
class RoomEvent;
class ServerInfo_User;
class ServerInfo_Room;
class ServerInfo_Game;
class Server_Game;
class Server;

class Server_Room : public QObject {
	Q_OBJECT
signals:
	void roomInfoChanged(ServerInfo_Room roomInfo);
	void gameListChanged(ServerInfo_Game gameInfo);
private:
	int id;
	QString name;
	QString description;
	bool autoJoin;
	QString joinMessage;
	QStringList gameTypes;
	QMap<int, Server_Game *> games;
	QMap<int, ServerInfo_Game> externalGames;
	QList<Server_ProtocolHandler *> userList;
	QMap<QString, ServerInfo_User_Container> externalUsers;
private slots:
	void broadcastGameListUpdate(ServerInfo_Game gameInfo, bool sendToIsl = true);
public:
	mutable QReadWriteLock usersLock;
	mutable QMutex gamesMutex;
	Server_Room(int _id, const QString &_name, const QString &_description, bool _autoJoin, const QString &_joinMessage, const QStringList &_gameTypes, Server *parent);
	~Server_Room();
	int getId() const { return id; }
	QString getName() const { return name; }
	QString getDescription() const { return description; }
	bool getAutoJoin() const { return autoJoin; }
	QString getJoinMessage() const { return joinMessage; }
	const QStringList &getGameTypes() const { return gameTypes; }
	const QMap<int, Server_Game *> &getGames() const { return games; }
	Server *getServer() const;
	ServerInfo_Room getInfo(bool complete, bool showGameTypes = false, bool updating = false, bool includeExternalData = true) const;
	int getGamesCreatedByUser(const QString &name) const;
	QList<ServerInfo_Game> getGamesOfUser(const QString &name) const;
	
	void addClient(Server_ProtocolHandler *client);
	void removeClient(Server_ProtocolHandler *client);
	
	void addExternalUser(const ServerInfo_User &userInfo);
	void removeExternalUser(const QString &name);
	const QMap<QString, ServerInfo_User_Container> &getExternalUsers() const { return externalUsers; }
	void updateExternalGameList(const ServerInfo_Game &gameInfo);
	
	void say(const QString &userName, const QString &s, bool sendToIsl = true);
	
	void addGame(Server_Game *game);
	void removeGame(Server_Game *game);
	
	void sendRoomEvent(RoomEvent *event, bool sendToIsl = true);
	RoomEvent *prepareRoomEvent(const ::google::protobuf::Message &roomEvent);
};

#endif
