#ifndef SERVER_ROOM_H
#define SERVER_ROOM_H

#include <QList>
#include <QMap>
#include <QObject>
#include <QStringList>
#include <QMutex>
#include <QReadWriteLock>
#include "serverinfo_user_container.h"
#include "pb/response.pb.h"
#include "pb/serverinfo_chat_message.pb.h"

class Server_DatabaseInterface;
class Server_ProtocolHandler;
class RoomEvent;
class ServerInfo_User;
class ServerInfo_Room;
class ServerInfo_Game;
class Server_Game;
class Server;

class Command_JoinGame;
class ResponseContainer;
class Server_AbstractUserInterface;

class Server_Room : public QObject {
    Q_OBJECT
signals:
    void roomInfoChanged(const ServerInfo_Room &roomInfo);
    void gameListChanged(const ServerInfo_Game &gameInfo);
private:
    int id;
    int chatHistorySize;
    QString name;
    QString description;
    QString permissionLevel;
    bool autoJoin;
    QString joinMessage;
    QStringList gameTypes;
    QMap<int, Server_Game *> games;
    QMap<int, ServerInfo_Game> externalGames;
    QMap<QString, Server_ProtocolHandler *> users;
    QMap<QString, ServerInfo_User_Container> externalUsers;
    QList<ServerInfo_ChatMessage> chatHistory;
private slots:
    void broadcastGameListUpdate(const ServerInfo_Game &gameInfo, bool sendToIsl = true);
public:
    mutable QReadWriteLock usersLock;
    mutable QReadWriteLock gamesLock;
    mutable QReadWriteLock historyLock;
    Server_Room(int _id, int _chatHistorySize, const QString &_name, const QString &_description, const QString &_permissionLevel, bool _autoJoin, const QString &_joinMessage, const QStringList &_gameTypes, Server *parent );
    ~Server_Room();
    int getId() const { return id; }
    QString getName() const { return name; }
    QString getDescription() const { return description; }
    QString getRoomPermission() const { return permissionLevel; }
    bool getAutoJoin() const { return autoJoin; }
    QString getJoinMessage() const { return joinMessage; }
    const QStringList &getGameTypes() const { return gameTypes; }
    const QMap<int, Server_Game *> &getGames() const { return games; }
    const QMap<int, ServerInfo_Game> &getExternalGames() const { return externalGames; }
    Server *getServer() const;
    const ServerInfo_Room &getInfo(ServerInfo_Room &result, bool complete, bool showGameTypes = false, bool includeExternalData = true) const;
    int getGamesCreatedByUser(const QString &name) const;
    QList<ServerInfo_Game> getGamesOfUser(const QString &name) const;
    QList<ServerInfo_ChatMessage> & getChatHistory() { return chatHistory; }
    
    void addClient(Server_ProtocolHandler *client);
    void removeClient(Server_ProtocolHandler *client);
    
    void addExternalUser(const ServerInfo_User &userInfo);
    void removeExternalUser(const QString &name);
    const QMap<QString, ServerInfo_User_Container> &getExternalUsers() const { return externalUsers; }
    void updateExternalGameList(const ServerInfo_Game &gameInfo);
    
    Response::ResponseCode processJoinGameCommand(const Command_JoinGame &cmd, ResponseContainer &rc, Server_AbstractUserInterface *userInterface);
    
    void say(const QString &userName, const QString &s, bool sendToIsl = true);
    
    void addGame(Server_Game *game);
    void removeGame(Server_Game *game);
    
    void sendRoomEvent(RoomEvent *event, bool sendToIsl = true);
    RoomEvent *prepareRoomEvent(const ::google::protobuf::Message &roomEvent);
};

#endif
