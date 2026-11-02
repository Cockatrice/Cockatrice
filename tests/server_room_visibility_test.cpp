#include <QCoreApplication>
#include <QHash>
#include <QSemaphore>
#include <QSet>
#include <QThread>
#include <functional>
#include <gtest/gtest.h>
#include <libcockatrice/network/server/remote/server.h>
#include <libcockatrice/network/server/remote/server_database_interface.h>
#include <libcockatrice/network/server/remote/server_protocolhandler.h>
#include <libcockatrice/network/server/remote/server_room.h>
#include <libcockatrice/protocol/pb/commands.pb.h>
#include <libcockatrice/protocol/pb/event_ignore_visibility_changed.pb.h>
#include <libcockatrice/protocol/pb/event_list_games.pb.h>
#include <libcockatrice/protocol/pb/isl_message.pb.h>
#include <libcockatrice/protocol/pb/response_join_room.pb.h>
#include <libcockatrice/protocol/pb/room_event.pb.h>
#include <libcockatrice/protocol/pb/server_message.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_game.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_user.pb.h>
#include <libcockatrice/protocol/pb/session_commands.pb.h>
#include <libcockatrice/rng/rng_abstract.h>
RNG_Abstract *rng = nullptr;
namespace
{
class DB : public Server_DatabaseInterface
{
public:
    QHash<QString, QSet<QString>> ignored;
    bool queriedFromWrongThread = false;
    QMap<QString, ServerInfo_User> getIgnoreList(const QString &name) override
    {
        if (QThread::currentThread() != thread()) {
            queriedFromWrongThread = true;
        }
        QMap<QString, ServerInfo_User> result;
        for (const auto &n : ignored.value(name)) {
            ServerInfo_User u;
            u.set_name(n.toStdString());
            result.insert(n, u);
        }
        return result;
    }
    bool isInIgnoreList(const QString &a, const QString &b) override
    {
        return ignored.value(a).contains(b);
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
        return {};
    }
};
class TestServer : public Server
{
public:
    explicit TestServer(DB *db)
    {
        setDatabaseInterface(db);
    }
    void insert(Server_Room *r)
    {
        addRoom(r);
    }
    void registerDatabase(QThread *thread, DB *database)
    {
        databaseInterfaces.insert(thread, database);
    }
};
class User : public Server_ProtocolHandler
{
public:
    User(Server *s, DB *db, const QString &name, uint32_t level) : Server_ProtocolHandler(s, db)
    {
        ServerInfo_User info;
        info.set_name(name.toStdString());
        info.set_user_level(level);
        setUserInfo(info);
        authState = PasswordRight;
    }
    QString getAddress() const override
    {
        return {};
    }
    QString getConnectionType() const override
    {
        return {};
    }
    Response reply;
    QList<RoomEvent> events;
    std::function<void()> onResponse;

protected:
    void transmitProtocolItem(const ServerMessage &item) override
    {
        if (item.message_type() == ServerMessage::RESPONSE) {
            reply.CopyFrom(item.response());
            if (onResponse) {
                onResponse();
            }
        } else if (item.message_type() == ServerMessage::ROOM_EVENT) {
            events.append(item.room_event());
        }
    }
};
TEST(RoomVisibility, JoinSnapshotHidesBlockerButKeepsOtherGamesAndStaff)
{
    DB db;
    db.ignored["Alice"].insert("Bob");
    TestServer server(&db);
    auto *room = new Server_Room(1, 0, "Room", "", "", "", false, "", {}, &server);
    server.insert(room);
    ServerInfo_Game blocked;
    blocked.set_game_id(12);
    blocked.set_room_id(1);
    blocked.set_player_count(1);
    blocked.mutable_creator_info()->set_name("Alice");
    room->updateExternalGameList(blocked);
    ServerInfo_Game other;
    other.set_game_id(13);
    other.set_room_id(1);
    other.set_player_count(1);
    other.mutable_creator_info()->set_name("Carol");
    room->updateExternalGameList(other);
    auto join = [&](User &user) {
        CommandContainer c;
        c.set_cmd_id(1);
        Command_JoinRoom j;
        j.set_room_id(1);
        c.add_session_command()->MutableExtension(Command_JoinRoom::ext)->CopyFrom(j);
        user.processCommandContainer(c);
    };
    User bob(&server, &db, "Bob", ServerInfo_User::IsUser | ServerInfo_User::IsRegistered);
    join(bob);
    ASSERT_TRUE(bob.reply.HasExtension(Response_JoinRoom::ext));
    const auto &roomInfo = bob.reply.GetExtension(Response_JoinRoom::ext).room_info();
    ASSERT_EQ(roomInfo.game_list_size(), 1);
    EXPECT_EQ(roomInfo.game_list(0).game_id(), 13);
    User mod(&server, &db, "Mod", ServerInfo_User::IsUser | ServerInfo_User::IsModerator);
    db.ignored["Alice"].insert("Mod");
    join(mod);
    ASSERT_TRUE(mod.reply.HasExtension(Response_JoinRoom::ext));
    EXPECT_EQ(mod.reply.GetExtension(Response_JoinRoom::ext).room_info().game_list_size(), 2);
}
TEST(RoomVisibility, JoinResponsePrecedesConcurrentUpdates)
{
    DB db;
    TestServer server(&db);
    auto *room = new Server_Room(1, 0, "Room", "", "", "", false, "", {}, &server);
    server.insert(room);
    User bob(&server, &db, "Bob", ServerInfo_User::IsUser | ServerInfo_User::IsRegistered);
    bob.onResponse = [&] {
        ServerInfo_Game game;
        game.set_game_id(17);
        game.set_room_id(1);
        game.set_player_count(1);
        game.mutable_creator_info()->set_name("Alice");
        room->updateExternalGameList(game);
        EXPECT_TRUE(bob.events.empty());
    };
    CommandContainer join;
    join.set_cmd_id(1);
    join.add_session_command()->MutableExtension(Command_JoinRoom::ext)->set_room_id(1);
    bob.processCommandContainer(join);
    ASSERT_EQ(bob.reply.GetExtension(Response_JoinRoom::ext).room_info().game_list_size(), 0);
    int gameEvents = 0;
    for (const auto &event : bob.events) {
        if (event.HasExtension(Event_ListGames::ext)) {
            ++gameEvents;
            EXPECT_EQ(event.GetExtension(Event_ListGames::ext).game_list(0).game_id(), 17);
        }
    }
    EXPECT_EQ(gameEvents, 1);
}

TEST(RoomVisibility, PoolThreadUsesItsOwnDatabaseForSnapshotAndPendingUpdates)
{
    DB serverDb;
    serverDb.ignored["Alice"].insert("Bob");
    DB poolDb;
    poolDb.ignored = serverDb.ignored;
    TestServer server(&serverDb);
    auto *room = new Server_Room(1, 0, "Room", "", "", "", false, "", {}, &server);
    server.insert(room);
    ServerInfo_Game initial;
    initial.set_game_id(21);
    initial.set_room_id(1);
    initial.set_player_count(1);
    initial.mutable_creator_info()->set_name("Alice");
    room->updateExternalGameList(initial);
    User bob(&server, &poolDb, "Bob", ServerInfo_User::IsUser | ServerInfo_User::IsRegistered);
    QThread worker;
    QSemaphore entered, resume, done;
    bool serverRegistryOwnsPoolDb = false;
    bob.onResponse = [&] {
        entered.release();
        resume.acquire();
    };
    QThread *mainThread = QThread::currentThread();
    poolDb.moveToThread(&worker);
    server.registerDatabase(&worker, &poolDb);
    bob.moveToThread(&worker);
    worker.start();
    CommandContainer join;
    join.set_cmd_id(1);
    join.add_session_command()->MutableExtension(Command_JoinRoom::ext)->set_room_id(1);
    QMetaObject::invokeMethod(
        &bob,
        [&] {
            serverRegistryOwnsPoolDb = server.getDatabaseInterface() == &poolDb;
            bob.processCommandContainer(join);
            done.release();
        },
        Qt::QueuedConnection);
    const bool enteredResponse = entered.tryAcquire(1, 10000);
    if (enteredResponse) {
        ServerInfo_Game pending = initial;
        pending.set_game_id(22);
        room->updateExternalGameList(pending);
        pending.set_game_id(23);
        pending.mutable_creator_info()->set_name("Carol");
        room->updateExternalGameList(pending);
    }
    resume.release();
    const bool completed = done.tryAcquire(1, 10000);
    Response joinReply = bob.reply;
    bool pingCompletedWhileRoomLocked = false;
    if (completed) {
        QSemaphore pingDone;
        room->usersLock.lockForWrite();
        QMetaObject::invokeMethod(
            &bob,
            [&] {
                bob.onResponse = {};
                CommandContainer ping;
                ping.set_cmd_id(2);
                ping.add_session_command()->MutableExtension(Command_Ping::ext);
                bob.processCommandContainer(ping);
                pingDone.release();
            },
            Qt::QueuedConnection);
        pingCompletedWhileRoomLocked = pingDone.tryAcquire(1, 2000);
        room->usersLock.unlock();
        if (!pingCompletedWhileRoomLocked) {
            pingDone.acquire();
        }
        QMetaObject::invokeMethod(
            &bob,
            [&] {
                bob.moveToThread(mainThread);
                poolDb.moveToThread(mainThread);
            },
            Qt::BlockingQueuedConnection);
    }
    worker.quit();
    worker.wait();
    ASSERT_TRUE(enteredResponse);
    ASSERT_TRUE(completed);
    EXPECT_TRUE(serverRegistryOwnsPoolDb);
    EXPECT_TRUE(pingCompletedWhileRoomLocked);
    ASSERT_TRUE(joinReply.HasExtension(Response_JoinRoom::ext));
    EXPECT_EQ(joinReply.GetExtension(Response_JoinRoom::ext).room_info().game_list_size(), 0);
    QList<int> received;
    for (const auto &event : bob.events) {
        if (event.HasExtension(Event_ListGames::ext)) {
            for (const auto &game : event.GetExtension(Event_ListGames::ext).game_list()) {
                received.append(game.game_id());
            }
        }
    }
    EXPECT_EQ(received, QList<int>({23}));
    EXPECT_FALSE(serverDb.queriedFromWrongThread);
    EXPECT_FALSE(poolDb.queriedFromWrongThread);
}

TEST(RoomVisibility, MainThreadHandlerUsesMainDatabaseInsteadOfWorkerOwnedDatabase)
{
    DB mainDb;
    mainDb.ignored["Alice"].insert("Bob");
    DB poolDb;
    TestServer server(&mainDb);
    QThread worker;
    QThread *mainThread = QThread::currentThread();
    poolDb.moveToThread(&worker);
    server.registerDatabase(&worker, &poolDb);
    worker.start();
    auto *room = new Server_Room(1, 0, "Room", "", "", "", false, "", {}, &server);
    server.insert(room);
    ServerInfo_Game game;
    game.set_game_id(31);
    game.set_room_id(1);
    game.set_player_count(1);
    game.mutable_creator_info()->set_name("Alice");
    room->updateExternalGameList(game);
    // Servatrice WebSocket handlers stay on the main thread but carry the
    // pointer to their assigned worker pool's database.
    User bob(&server, &poolDb, "Bob", ServerInfo_User::IsUser | ServerInfo_User::IsRegistered);
    CommandContainer join;
    join.set_cmd_id(1);
    join.add_session_command()->MutableExtension(Command_JoinRoom::ext)->set_room_id(1);
    bob.processCommandContainer(join);
    const bool hasJoin = bob.reply.HasExtension(Response_JoinRoom::ext);
    const int visible = hasJoin ? bob.reply.GetExtension(Response_JoinRoom::ext).room_info().game_list_size() : -1;
    QMetaObject::invokeMethod(&poolDb, [&] { poolDb.moveToThread(mainThread); }, Qt::BlockingQueuedConnection);
    worker.quit();
    worker.wait();
    EXPECT_TRUE(hasJoin);
    EXPECT_EQ(visible, 0);
    EXPECT_FALSE(mainDb.queriedFromWrongThread);
    EXPECT_FALSE(poolDb.queriedFromWrongThread);
}

TEST(RoomVisibility, UpdatesIgnoreChangesAndRemoteMembershipStayCoherent)
{
    DB db;
    db.ignored["Alice"].insert("Bob");
    TestServer server(&db);
    auto *room = new Server_Room(1, 0, "Room", "", "", "", false, "", {}, &server);
    server.insert(room);
    ServerInfo_Game game;
    game.set_game_id(12);
    game.set_room_id(1);
    game.set_player_count(1);
    game.mutable_creator_info()->set_name("Alice");
    room->updateExternalGameList(game);
    User bob(&server, &db, "Bob", ServerInfo_User::IsUser | ServerInfo_User::IsRegistered);
    CommandContainer join;
    join.set_cmd_id(1);
    join.add_session_command()->MutableExtension(Command_JoinRoom::ext)->set_room_id(1);
    bob.processCommandContainer(join);
    ASSERT_EQ(bob.reply.GetExtension(Response_JoinRoom::ext).room_info().game_list_size(), 0);
    bob.events.clear();

    ServerInfo_Game partial;
    partial.set_game_id(12);
    partial.set_room_id(1);
    partial.set_started(true);
    room->updateExternalGameList(partial);
    EXPECT_TRUE(bob.events.empty());
    ASSERT_TRUE(room->getExternalGames().contains(12));
    EXPECT_TRUE(room->getExternalGames().value(12).started());

    QList<IslMessage> messages;
    QObject::connect(&server, &Server::sigSendIslMessage, &server,
                     [&](const IslMessage &message, int) { messages.append(message); });
    db.ignored["Alice"].remove("Bob");
    server.notifyIgnoreVisibilityChanged("Alice", "Bob");
    ASSERT_EQ(messages.size(), 1);
    ASSERT_TRUE(messages.first().session_event().HasExtension(Event_IgnoreVisibilityChanged::ext));
    const auto &invalidation = messages.first().session_event().GetExtension(Event_IgnoreVisibilityChanged::ext);
    EXPECT_EQ(invalidation.owner(), "Alice");
    EXPECT_EQ(invalidation.viewer(), "Bob");
    ASSERT_EQ(bob.events.size(), 1);
    auto shown = bob.events.takeLast().GetExtension(Event_ListGames::ext).game_list(0);
    EXPECT_FALSE(shown.closed());
    EXPECT_TRUE(shown.started());
    EXPECT_EQ(shown.creator_info().name(), "Alice");

    db.ignored["Alice"].insert("Bob");
    server.refreshIgnoreVisibility("Alice", "Bob");
    ASSERT_EQ(bob.events.size(), 1);
    EXPECT_TRUE(bob.events.takeLast().GetExtension(Event_ListGames::ext).game_list(0).closed());

    bob.playerAddedToGame(12, 1, 5);
    QCoreApplication::processEvents();
    ASSERT_EQ(bob.events.size(), 1);
    EXPECT_FALSE(bob.events.takeLast().GetExtension(Event_ListGames::ext).game_list(0).closed());
    bob.remotePlayerRemovedFromGame(12, 1);
    QCoreApplication::processEvents();
    ASSERT_EQ(bob.events.size(), 1);
    EXPECT_TRUE(bob.events.takeLast().GetExtension(Event_ListGames::ext).game_list(0).closed());

    ServerInfo_Game closed;
    closed.set_game_id(12);
    closed.set_room_id(1);
    closed.set_closed(true);
    room->updateExternalGameList(closed);
    EXPECT_TRUE(bob.events.empty());
    db.ignored["Alice"].remove("Bob");
    server.refreshIgnoreVisibility("Alice", "Bob");
    EXPECT_TRUE(bob.events.empty());
}
} // namespace
int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
