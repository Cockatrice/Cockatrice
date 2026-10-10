#ifndef SERVER_ROOM_H
#define SERVER_ROOM_H

#include <QList>
#include <QMap>
#include <QObject>
#include <QReadWriteLock>
#include <QString>
#include <QStringList>
#include <libcockatrice/protocol/pb/response.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_chat_message.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_room_channel.pb.h>
#include <qtmetamacros.h>

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
class ServerInfo_User_Container;
namespace google
{
namespace protobuf
{
class Message;
} // namespace protobuf
} // namespace google

class Server_Room : public QObject
{
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
    QString privilegeLevel;
    bool autoJoin;
    QString joinMessage;
    QStringList gameTypes;
    QList<ServerInfo_RoomChannel> channels;
    QMap<QString, ServerInfo_RoomChannel> channelIndex;
    QMap<int, Server_Game *> games;
    QMap<int, ServerInfo_Game> externalGames;
    QMap<QString, Server_ProtocolHandler *> users;
    QMap<QString, ServerInfo_User_Container> externalUsers;
    QMap<QString, QList<ServerInfo_ChatMessage>> channelChatHistory;
private slots:
    void broadcastGameListUpdate(const ServerInfo_Game &gameInfo, bool sendToIsl = true);

public:
    static const QString MAIN_CHANNEL_KEY;

    mutable QReadWriteLock usersLock;
    mutable QReadWriteLock gamesLock;
    mutable QReadWriteLock historyLock;
    Server_Room(int _id,
                int _chatHistorySize,
                const QString &_name,
                const QString &_description,
                const QString &_permissionLevel,
                const QString &_privilegeLevel,
                bool _autoJoin,
                const QString &_joinMessage,
                const QStringList &_gameTypes,
                Server *parent,
                const QList<ServerInfo_RoomChannel> &_channels = {});
    ~Server_Room() override;
    int getId() const
    {
        return id;
    }
    QString getName() const
    {
        return name;
    }
    QString getDescription() const
    {
        return description;
    }
    QString getRoomPermission() const
    {
        return permissionLevel;
    }
    QString getRoomPrivilege() const
    {
        return privilegeLevel;
    }
    bool getAutoJoin() const
    {
        return autoJoin;
    }
    bool userMayJoin(const ServerInfo_User &userInfo);
    QString getJoinMessage() const
    {
        return joinMessage;
    }
    const QStringList &getGameTypes() const
    {
        return gameTypes;
    }
    const QList<ServerInfo_RoomChannel> &getChannels() const
    {
        return channels;
    }
    /**
     * @brief Looks up a catalog channel by its stable id.
     * @return The matching channel, or nullptr when the id is not in the catalog.
     */
    const ServerInfo_RoomChannel *findChannel(const QString &channelId) const;
    /** @brief True when @p channelId refers to the implicit Main channel. */
    static bool isMainChannel(const QString &channelId);
    const QMap<int, Server_Game *> &getGames() const
    {
        return games;
    }
    const QMap<int, ServerInfo_Game> &getExternalGames() const
    {
        return externalGames;
    }
    Server *getServer() const;
    const ServerInfo_Room &
    getInfo(ServerInfo_Room &result, bool complete, bool showGameTypes = false, bool includeExternalData = true) const;
    int getGamesCreatedByUser(const QString &name) const;
    QList<ServerInfo_Game> getGamesOfUser(const QString &name) const;
    QList<ServerInfo_ChatMessage> getChatHistory(const QString &channelId) const;

    void addClient(Server_ProtocolHandler *client);
    void removeClient(Server_ProtocolHandler *client);

    void addExternalUser(const ServerInfo_User &userInfo);
    void removeExternalUser(const QString &_name);
    const QMap<QString, ServerInfo_User_Container> &getExternalUsers() const
    {
        return externalUsers;
    }
    void updateExternalGameList(const ServerInfo_Game &gameInfo);

    Response::ResponseCode processJoinGameCommand(const Command_JoinGame &cmd,
                                                  ResponseContainer &rc,
                                                  Server_AbstractUserInterface *userInterface);

    void say(const QString &userName, const QString &s, const QString &channelId = QString(), bool sendToIsl = true);
    void removeSaidMessages(const QString &userName,
                            int amount,
                            bool sendToIsl = true,
                            const QString &channelId = QString());

    void addGame(Server_Game *game);
    void removeGame(Server_Game *game);

    void sendRoomEvent(RoomEvent *event, bool sendToIsl = true);
    RoomEvent *prepareRoomEvent(const ::google::protobuf::Message &roomEvent);
};

#endif
