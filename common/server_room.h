#ifndef SERVER_ROOM_H
#define SERVER_ROOM_H

#include <QList>
#include <QMap>
#include <QObject>
#include <QStringList>

class Server_ProtocolHandler;
class RoomEvent;
class ServerInfo_User;
class ServerInfo_Room;
class Server_Game;
class Server;

class Server_Room : public QObject, public QList<Server_ProtocolHandler *> {
	Q_OBJECT
signals:
	void roomInfoChanged();
	void gameCreated(Server_Game *game);
	void gameClosing(int gameId);
private:
	int id;
	QString name;
	QString description;
	bool autoJoin;
	QString joinMessage;
	QMap<int, Server_Game *> games;
private slots:
	void removeGame();
public:
	Server_Room(int _id, const QString &_name, const QString &_description, bool _autoJoin, const QString &_joinMessage, Server *parent);
	int getId() const { return id; }
	QString getName() const { return name; }
	QString getDescription() const { return description; }
	bool getAutoJoin() const { return autoJoin; }
	QString getJoinMessage() const { return joinMessage; }
	const QMap<int, Server_Game *> &getGames() const { return games; }
	Server *getServer() const;
	ServerInfo_Room *getInfo(bool complete) const;
	
	void addClient(Server_ProtocolHandler *client);
	void removeClient(Server_ProtocolHandler *client);
	void say(Server_ProtocolHandler *client, const QString &s);
	void broadcastGameListUpdate(Server_Game *game);
	Server_Game *createGame(const QString &description, const QString &password, int maxPlayers, bool spectatorsAllowed, bool spectatorsNeedPassword, bool spectatorsCanTalk, bool spectatorsSeeEverything, Server_ProtocolHandler *creator);
	
	void sendRoomEvent(RoomEvent *event);
};

#endif
