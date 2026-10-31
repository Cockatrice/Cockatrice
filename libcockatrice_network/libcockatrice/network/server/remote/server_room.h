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
    /**
     * @brief True when the flags in @p userInfo satisfy @p requiredLevel.
     *
     * User flags are independent bits, not a ladder: IsDeveloper does not
     * imply IsModerator or IsAdmin, so developer-only users cannot read or
     * post in moderator/admin channels. Admins hold the moderator bit
     * alongside the admin bit and moderators are admitted to judge channels.
     */
    static bool hasChannelPrivilege(const ServerInfo_User &userInfo, ServerInfo_RoomChannel::AccessLevel requiredLevel);
    /** @brief True when @p userInfo may read @p channel. */
    static bool mayReadChannel(const ServerInfo_User &userInfo, const ServerInfo_RoomChannel &channel);
    /**
     * @brief True when @p userInfo may write to @p channel.
     *
     * Writing requires the channel's write level, or — when no explicit write
     * level is configured — the same level as reading.
     */
    static bool mayWriteChannel(const ServerInfo_User &userInfo, const ServerInfo_RoomChannel &channel);
    const QMap<int, Server_Game *> &getGames() const
    {
        return games;
    }
    const QMap<int, ServerInfo_Game> &getExternalGames() const
    {
        return externalGames;
    }
    Server *getServer() const;
    const ServerInfo_Room &getInfo(ServerInfo_Room &result,
                                   bool complete,
                                   bool showGameTypes = false,
                                   bool includeExternalData = true,
                                   const ServerInfo_User *forUser = nullptr) const;
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
    void removeSaidMessages(const QString &userName, int amount, bool sendToIsl = true);

    void addGame(Server_Game *game);
    void removeGame(Server_Game *game);

    void sendRoomEvent(RoomEvent *event,
                       bool sendToIsl = true,
                       ServerInfo_RoomChannel::AccessLevel accessLevel = ServerInfo_RoomChannel::Public);
    RoomEvent *prepareRoomEvent(const ::google::protobuf::Message &roomEvent);
};

#endif
