/** @file server_room_channels_test.cpp
 *  @brief Tests for room chat channels: fan-out, privilege gating, whitelist, history replay and audit logging.
 *  @ingroup Tests
 */

#include "libcockatrice/protocol/pb/response.pb.h"
#include "libcockatrice/protocol/pb/server_message.pb.h"
#include "server.h"
#include "server_database_interface.h"
#include "server_protocolhandler.h"
#include "server_room.h"

#include <QList>
#include <QObject>
#include <QString>
#include <gtest/gtest.h>
#include <libcockatrice/protocol/get_pb_extension.h>
#include <libcockatrice/protocol/pb/commands.pb.h>
#include <libcockatrice/protocol/pb/event_room_channel_say.pb.h>
#include <libcockatrice/protocol/pb/event_room_say.pb.h>
#include <libcockatrice/protocol/pb/isl_message.pb.h>
#include <libcockatrice/protocol/pb/room_commands.pb.h>
#include <libcockatrice/protocol/pb/room_event.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_chat_message.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_room_channel.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_user.pb.h>
#include <libcockatrice/protocol/pb/session_commands.pb.h>
#include <libcockatrice/rng/rng_abstract.h>
#include <string>

RNG_Abstract *rng = nullptr; // referenced by the server_remote library

namespace
{

class MockDatabaseInterface : public Server_DatabaseInterface
{
public:
    struct LogEntry
    {
        QString senderName;
        QString message;
        LogMessage_TargetType targetType;
        int targetId;
        QString targetName;
        QString channel;
    };
    QList<LogEntry> logEntries;

    void logMessage(const int /* senderId */,
                    const QString &senderName,
                    const QString & /* senderIp */,
                    const QString &message,
                    LogMessage_TargetType targetType,
                    const int targetId,
                    const QString &targetName,
                    const QString &channel = QString()) override
    {
        logEntries.append({senderName, message, targetType, targetId, targetName, channel});
    }

    AuthenticationResult checkUserPassword(Server_ProtocolHandler *,
                                           const QString &,
                                           const QString &,
                                           const QString &,
                                           QString &,
                                           int &,
                                           bool) override
    {
        return NotLoggedIn;
    }
    int getNextReplayId() override
    {
        return 1;
    }
    int getNextGameId() override
    {
        return 1;
    }
    int getActiveUserCount(QString) override
    {
        return 0;
    }
    ServerInfo_User getUserData(const QString &, bool) override
    {
        return ServerInfo_User();
    }
};

class FakeServer : public Server
{
public:
    explicit FakeServer(Server_DatabaseInterface *db) : Server()
    {
        setDatabaseInterface(db);
    }
    void insertRoom(Server_Room *newRoom)
    {
        addRoom(newRoom);
    }
};

class TestRoomHandler : public Server_ProtocolHandler
{
public:
    TestRoomHandler(Server *_server, Server_DatabaseInterface *_db) : Server_ProtocolHandler(_server, _db)
    {
    }

    QString getAddress() const override
    {
        return {};
    }
    QString getConnectionType() const override
    {
        return {};
    }

    void authenticate(const ServerInfo_User &userInfo_)
    {
        setUserInfo(userInfo_);
        authState = PasswordRight;
    }
    void attachToRoom(Server_Room *targetRoom)
    {
        rooms.insert(targetRoom->getId(), targetRoom);
    }

    QList<RoomEvent> receivedRoomEvents;
    Response::ResponseCode lastResponseCode = Response::RespNothing;

    void clearReceived()
    {
        receivedRoomEvents.clear();
        lastResponseCode = Response::RespNothing;
    }

protected:
    void transmitProtocolItem(const ServerMessage &item) override
    {
        if (item.message_type() == ServerMessage::ROOM_EVENT) {
            receivedRoomEvents.append(item.room_event());
        } else if (item.message_type() == ServerMessage::RESPONSE) {
            lastResponseCode = item.response().response_code();
        }
    }
};

class RoomChannelTest : public ::testing::Test
{
protected:
    MockDatabaseInterface db;
    FakeServer server{&db};
    QList<ServerInfo_RoomChannel> catalog;
    Server_Room *room = nullptr;
    TestRoomHandler mod{&server, &db};
    TestRoomHandler adminUser{&server, &db};
    TestRoomHandler user{&server, &db};
    ServerInfo_User moderator;
    ServerInfo_User admin;
    ServerInfo_User plainUser;

    void SetUp() override
    {
        ServerInfo_RoomChannel lobby;
        lobby.set_id("lobby");
        lobby.set_display_name("Lobby");
        lobby.set_access_level(ServerInfo_RoomChannel::Public);
        catalog.append(lobby);

        ServerInfo_RoomChannel mods;
        mods.set_id("mods");
        mods.set_display_name("Moderators");
        mods.set_access_level(ServerInfo_RoomChannel::Moderator);
        catalog.append(mods);

        ServerInfo_RoomChannel ops;
        ops.set_id("ops");
        ops.set_display_name("Admins");
        ops.set_access_level(ServerInfo_RoomChannel::Admin);
        catalog.append(ops);

        room = new Server_Room(1, 1000, "test room", "", "", "", false, "Welcome", QStringList(), &server, catalog);

        moderator.set_name("mod");
        moderator.set_user_level(ServerInfo_User::IsUser | ServerInfo_User::IsRegistered |
                                 ServerInfo_User::IsModerator);
        // Deliberately carries no IsModerator flag: an admin reaches moderator
        // channels through its privilege rank, not by holding the moderator bit.
        admin.set_name("admin");
        admin.set_user_level(ServerInfo_User::IsUser | ServerInfo_User::IsRegistered | ServerInfo_User::IsAdmin);
        plainUser.set_name("user");
        plainUser.set_user_level(ServerInfo_User::IsUser | ServerInfo_User::IsRegistered);
    }

    void TearDown() override
    {
        delete room;
    }

    // Make @p handler a member of the room directly, without history replay or JOIN_ROOM bookkeeping.
    void joinMember(TestRoomHandler &handler, const ServerInfo_User &userInfo_)
    {
        handler.authenticate(userInfo_);
        handler.attachToRoom(room);
        room->addClient(&handler);
    }

    void sendMainSay(TestRoomHandler &handler, const QString &message)
    {
        CommandContainer cont;
        cont.set_cmd_id(1);
        cont.set_room_id(room->getId());
        Command_RoomSay cmd;
        cmd.set_message(message.toStdString());
        cont.add_room_command()->MutableExtension(Command_RoomSay::ext)->CopyFrom(cmd);
        handler.processCommandContainer(cont);
    }

    void sendChannelSay(TestRoomHandler &handler, const QString &channelId, const QString &message)
    {
        CommandContainer cont;
        cont.set_cmd_id(1);
        cont.set_room_id(room->getId());
        Command_RoomChannelSay cmd;
        cmd.set_channel_id(channelId.toStdString());
        cmd.set_message(message.toStdString());
        cont.add_room_command()->MutableExtension(Command_RoomChannelSay::ext)->CopyFrom(cmd);
        handler.processCommandContainer(cont);
    }

    void joinThroughCommand(TestRoomHandler &handler)
    {
        CommandContainer cont;
        cont.set_cmd_id(1);
        Command_JoinRoom cmd;
        cmd.set_room_id(room->getId());
        cont.add_session_command()->MutableExtension(Command_JoinRoom::ext)->CopyFrom(cmd);
        handler.processCommandContainer(cont);
    }

    static QList<RoomEvent> channelSayEvents(const TestRoomHandler &handler)
    {
        QList<RoomEvent> result;
        for (const RoomEvent &event : handler.receivedRoomEvents) {
            if (static_cast<RoomEvent::RoomEventType>(getPbExtension(event)) == RoomEvent::ROOM_CHANNEL_SAY) {
                result.append(event);
            }
        }
        return result;
    }

    static QList<RoomEvent> roomSayEvents(const TestRoomHandler &handler)
    {
        QList<RoomEvent> result;
        for (const RoomEvent &event : handler.receivedRoomEvents) {
            if (static_cast<RoomEvent::RoomEventType>(getPbExtension(event)) == RoomEvent::ROOM_SAY) {
                result.append(event);
            }
        }
        return result;
    }

    static QString channelIdOf(const RoomEvent &event)
    {
        return QString::fromStdString(event.GetExtension(Event_RoomChannelSay::ext).channel_id());
    }

    static QString messageOf(const RoomEvent &event)
    {
        return QString::fromStdString(event.GetExtension(Event_RoomChannelSay::ext).message());
    }

    static QString mainMessageOf(const RoomEvent &event)
    {
        return QString::fromStdString(event.GetExtension(Event_RoomSay::ext).message());
    }
};

TEST_F(RoomChannelTest, MainSayStillBroadcastsToAllRoomMembers)
{
    joinMember(mod, moderator);
    joinMember(user, plainUser);
    mod.clearReceived();
    user.clearReceived();

    sendMainSay(user, "hello everyone");

    EXPECT_EQ(user.lastResponseCode, Response::RespOk);
    EXPECT_EQ(roomSayEvents(mod).size(), 1);
    EXPECT_EQ(mainMessageOf(roomSayEvents(mod).first()), "hello everyone");
    EXPECT_EQ(roomSayEvents(user).size(), 1);
    EXPECT_EQ(mainMessageOf(roomSayEvents(user).first()), "hello everyone");
    EXPECT_TRUE(channelSayEvents(mod).isEmpty());
    EXPECT_TRUE(channelSayEvents(user).isEmpty());
}

TEST_F(RoomChannelTest, PublicChannelSaysBroadcastToEveryone)
{
    joinMember(mod, moderator);
    joinMember(user, plainUser);
    mod.clearReceived();
    user.clearReceived();

    sendChannelSay(user, "lobby", "is there a game tonight?");

    EXPECT_EQ(user.lastResponseCode, Response::RespOk);
    EXPECT_EQ(channelSayEvents(mod).size(), 1);
    EXPECT_EQ(channelIdOf(channelSayEvents(mod).first()), "lobby");
    EXPECT_EQ(messageOf(channelSayEvents(mod).first()), "is there a game tonight?");
    EXPECT_EQ(channelSayEvents(user).size(), 1);
}

TEST_F(RoomChannelTest, ModeratorChannelReachesModeratorAndAbove)
{
    joinMember(mod, moderator);
    joinMember(adminUser, admin);
    joinMember(user, plainUser);
    mod.clearReceived();
    adminUser.clearReceived();
    user.clearReceived();

    sendChannelSay(mod, "mods", "quiet channel");

    EXPECT_EQ(mod.lastResponseCode, Response::RespOk);
    EXPECT_EQ(channelSayEvents(mod).size(), 1);
    EXPECT_EQ(channelIdOf(channelSayEvents(mod).first()), "mods");
    // An admin outranks a moderator and so also receives the channel message.
    EXPECT_EQ(channelSayEvents(adminUser).size(), 1);
    EXPECT_TRUE(channelSayEvents(user).isEmpty());
    EXPECT_TRUE(roomSayEvents(user).isEmpty());
}

TEST_F(RoomChannelTest, AdminReachesModeratorChannelWithoutModeratorFlag)
{
    joinMember(mod, moderator);
    joinMember(adminUser, admin);
    mod.clearReceived();
    adminUser.clearReceived();

    sendChannelSay(adminUser, "mods", "admin checking in");

    EXPECT_EQ(adminUser.lastResponseCode, Response::RespOk);
    EXPECT_EQ(channelSayEvents(mod).size(), 1);
    EXPECT_EQ(channelSayEvents(adminUser).size(), 1);
    EXPECT_EQ(messageOf(channelSayEvents(mod).first()), "admin checking in");
}

TEST_F(RoomChannelTest, UserBelowRequiredLevelCannotSendToChannel)
{
    joinMember(mod, moderator);
    joinMember(user, plainUser);
    mod.clearReceived();
    user.clearReceived();

    sendChannelSay(user, "mods", "let me in");

    EXPECT_EQ(user.lastResponseCode, Response::RespFunctionNotAllowed);
    EXPECT_TRUE(channelSayEvents(mod).isEmpty());
    EXPECT_TRUE(channelSayEvents(user).isEmpty());
    bool loggedAsChannel = false;
    for (const MockDatabaseInterface::LogEntry &entry : db.logEntries) {
        loggedAsChannel |= entry.targetType == Server_DatabaseInterface::MessageTargetRoomChannel;
    }
    EXPECT_FALSE(loggedAsChannel);
}

TEST_F(RoomChannelTest, AdminChannelRejectsModerator)
{
    joinMember(mod, moderator);
    joinMember(adminUser, admin);
    mod.clearReceived();
    adminUser.clearReceived();

    sendChannelSay(mod, "ops", "not allowed");

    EXPECT_EQ(mod.lastResponseCode, Response::RespFunctionNotAllowed);
    EXPECT_TRUE(channelSayEvents(mod).isEmpty());
    EXPECT_TRUE(channelSayEvents(adminUser).isEmpty());
}

TEST_F(RoomChannelTest, AdminChannelReachesAdminsOnly)
{
    joinMember(mod, moderator);
    joinMember(adminUser, admin);
    joinMember(user, plainUser);
    mod.clearReceived();
    adminUser.clearReceived();
    user.clearReceived();

    sendChannelSay(adminUser, "ops", "staff only");

    EXPECT_EQ(adminUser.lastResponseCode, Response::RespOk);
    EXPECT_EQ(channelSayEvents(adminUser).size(), 1);
    EXPECT_TRUE(channelSayEvents(mod).isEmpty());
    EXPECT_TRUE(channelSayEvents(user).isEmpty());
}

TEST_F(RoomChannelTest, UnknownChannelIdIsRejected)
{
    joinMember(user, plainUser);
    user.clearReceived();

    sendChannelSay(user, "not-a-channel", "spam");

    EXPECT_EQ(user.lastResponseCode, Response::RespInvalidCommand);
    EXPECT_TRUE(channelSayEvents(user).isEmpty());
    EXPECT_TRUE(db.logEntries.isEmpty());
}

TEST_F(RoomChannelTest, MissingAccessLevelDefaultsToPublic)
{
    ServerInfo_RoomChannel defaultAccess;
    defaultAccess.set_id("public-by-default");
    defaultAccess.set_display_name("Default Access");
    catalog.append(defaultAccess);
    auto *secondRoom = new Server_Room(2, 1000, "second", "", "", "", false, "", QStringList(), &server, catalog);

    TestRoomHandler joiner{&server, &db};
    joiner.authenticate(plainUser);
    joiner.attachToRoom(secondRoom);
    secondRoom->addClient(&joiner);

    CommandContainer cont;
    cont.set_cmd_id(1);
    cont.set_room_id(2);
    Command_RoomChannelSay cmd;
    cmd.set_channel_id("public-by-default");
    cmd.set_message("hi");
    cont.add_room_command()->MutableExtension(Command_RoomChannelSay::ext)->CopyFrom(cmd);
    joiner.processCommandContainer(cont);

    EXPECT_EQ(joiner.lastResponseCode, Response::RespOk);
    EXPECT_EQ(channelSayEvents(joiner).size(), 1);

    delete secondRoom;
}

TEST_F(RoomChannelTest, HistoryStoredPerChannel)
{
    room->say("user", "main message", QString(), false);
    room->say("user", "lobby message", "lobby", false);
    room->say("user", "mods message", "mods", false);

    const QList<ServerInfo_ChatMessage> mainHistory = room->getChatHistory(Server_Room::MAIN_CHANNEL_KEY);
    EXPECT_EQ(mainHistory.size(), 1);
    EXPECT_EQ(QString::fromStdString(mainHistory.first().message()), "main message");
    EXPECT_TRUE(QString::fromStdString(mainHistory.first().channel()).isEmpty());

    const QList<ServerInfo_ChatMessage> lobbyHistory = room->getChatHistory("lobby");
    EXPECT_EQ(lobbyHistory.size(), 1);
    EXPECT_EQ(QString::fromStdString(lobbyHistory.first().message()), "lobby message");
    EXPECT_EQ(QString::fromStdString(lobbyHistory.first().channel()), "lobby");

    const QList<ServerInfo_ChatMessage> modsHistory = room->getChatHistory("mods");
    EXPECT_EQ(modsHistory.size(), 1);
    EXPECT_EQ(QString::fromStdString(modsHistory.first().message()), "mods message");
    EXPECT_EQ(QString::fromStdString(modsHistory.first().channel()), "mods");

    // A channel id maps to its own history bucket, never to the main one.
    EXPECT_EQ(room->getChatHistory("").size(), mainHistory.size());
}

TEST_F(RoomChannelTest, JoinReplaysOnlyVisibleChannelHistory)
{
    server.insertRoom(room);
    joinMember(mod, moderator);
    joinMember(user, plainUser);

    room->say("user", "main message", QString(), false);
    room->say("user", "lobby message", "lobby", false);
    room->say("mod", "mods message", "mods", false);
    room->say("admin", "ops message", "ops", false);

    TestRoomHandler lateUser{&server, &db};
    lateUser.authenticate(plainUser);
    joinThroughCommand(lateUser);

    EXPECT_EQ(lateUser.lastResponseCode, Response::RespOk);
    const QList<RoomEvent> lateChannelEvents = channelSayEvents(lateUser);
    // Replay: one main history event, one public channel history event, the welcome message -- but no restricted
    // history.
    EXPECT_EQ(roomSayEvents(lateUser).size(), 2);
    EXPECT_EQ(lateChannelEvents.size(), 1);
    EXPECT_EQ(channelIdOf(lateChannelEvents.first()), "lobby");

    TestRoomHandler lateMod{&server, &db};
    lateMod.authenticate(moderator);
    joinThroughCommand(lateMod);

    EXPECT_EQ(lateMod.lastResponseCode, Response::RespOk);
    const QList<RoomEvent> lateModChannelEvents = channelSayEvents(lateMod);
    EXPECT_EQ(lateModChannelEvents.size(), 2);
    EXPECT_EQ(channelIdOf(lateModChannelEvents.at(0)), "lobby");
    EXPECT_EQ(channelIdOf(lateModChannelEvents.at(1)), "mods");

    TestRoomHandler lateAdmin{&server, &db};
    lateAdmin.authenticate(admin);
    joinThroughCommand(lateAdmin);

    EXPECT_EQ(lateAdmin.lastResponseCode, Response::RespOk);
    const QList<RoomEvent> lateAdminChannelEvents = channelSayEvents(lateAdmin);
    // An admin sees the moderator channel and the admin channel on top of the public one.
    EXPECT_EQ(lateAdminChannelEvents.size(), 3);
    EXPECT_EQ(channelIdOf(lateAdminChannelEvents.at(0)), "lobby");
    EXPECT_EQ(channelIdOf(lateAdminChannelEvents.at(1)), "mods");
    EXPECT_EQ(channelIdOf(lateAdminChannelEvents.at(2)), "ops");
}

TEST_F(RoomChannelTest, AuditLoggingTagsChannelMessages)
{
    joinMember(user, plainUser);

    sendMainSay(user, "main log line");
    sendChannelSay(user, "lobby", "channel log line");

    ASSERT_EQ(db.logEntries.size(), 2);
    EXPECT_EQ(db.logEntries.at(0).targetType, Server_DatabaseInterface::MessageTargetRoom);
    EXPECT_TRUE(db.logEntries.at(0).channel.isEmpty());
    EXPECT_EQ(db.logEntries.at(1).targetType, Server_DatabaseInterface::MessageTargetRoomChannel);
    EXPECT_EQ(db.logEntries.at(1).channel, "lobby");
    EXPECT_EQ(db.logEntries.at(1).senderName, "user");
    EXPECT_EQ(db.logEntries.at(1).message, "channel log line");
}

TEST_F(RoomChannelTest, PublicChannelRelaysOverIslButRestrictedChannelsDoNot)
{
    QList<IslMessage> relMessages;
    QObject::connect(&server, &Server::sigSendIslMessage,
                     [&relMessages](const IslMessage &message, int) { relMessages.append(message); });

    room->say("mod", "public msg", "lobby");
    room->say("mod", "mods msg", "mods");
    room->say("admin", "ops msg", "ops");

    ASSERT_EQ(relMessages.size(), 1);
    const RoomEvent &relayed = relMessages.first().room_event();
    EXPECT_EQ(static_cast<RoomEvent::RoomEventType>(getPbExtension(relayed)), RoomEvent::ROOM_CHANNEL_SAY);
    EXPECT_EQ(channelIdOf(relayed), "lobby");
    EXPECT_EQ(messageOf(relayed), "public msg");
}

} // namespace

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}