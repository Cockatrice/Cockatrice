/** @file server_forced_password_change_test.cpp
 *  @brief Tests for the restricted session a forced password change leaves behind at login.
 *  @ingroup Tests
 */

#include "libcockatrice/protocol/pb/response.pb.h"
#include "libcockatrice/protocol/pb/server_message.pb.h"
#include "server.h"
#include "server_database_interface.h"
#include "server_protocolhandler.h"

#include <QString>
#include <gtest/gtest.h>
#include <libcockatrice/protocol/pb/commands.pb.h>
#include <libcockatrice/protocol/pb/room_commands.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_user.pb.h>
#include <libcockatrice/protocol/pb/session_commands.pb.h>
#include <libcockatrice/rng/rng_abstract.h>

RNG_Abstract *rng = nullptr; // referenced by the server_remote library

namespace
{

class MockDatabaseInterface : public Server_DatabaseInterface
{
public:
    AuthenticationResult checkUserPassword(Server_ProtocolHandler *,
                                           const QString &,
                                           const QString &,
                                           const QString &,
                                           QString &,
                                           int &,
                                           bool) override
    {
        return PasswordChangeRequired;
    }
    ServerInfo_User getUserData(const QString &name, bool withId) override
    {
        ServerInfo_User user;
        user.set_name(name.toStdString());
        user.set_user_level(ServerInfo_User::IsUser | ServerInfo_User::IsRegistered | ServerInfo_User::IsModerator);
        if (withId) {
            user.set_id(1);
        }
        return user;
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
};

class FakeServer : public Server
{
public:
    FakeServer()
    {
        setDatabaseInterface(new MockDatabaseInterface());
    }
};

class TestHandler : public Server_ProtocolHandler
{
public:
    TestHandler(Server *_server, Server_DatabaseInterface *_db) : Server_ProtocolHandler(_server, _db)
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

    Response::ResponseCode lastResponseCode = Response::RespNothing;
    int lastExtendedSessionCommand = -1;

protected:
    Response::ResponseCode
    processExtendedSessionCommand(int cmdType, const SessionCommand & /*cmd*/, ResponseContainer & /*rc*/) override
    {
        lastExtendedSessionCommand = cmdType;
        return Response::RespOk;
    }

    void transmitProtocolItem(const ServerMessage &item) override
    {
        if (item.message_type() == ServerMessage::RESPONSE) {
            lastResponseCode = item.response().response_code();
        }
    }
};

class ForcedPasswordChangeTest : public ::testing::Test
{
protected:
    FakeServer server;
    TestHandler handler{&server, server.getDatabaseInterface()};

    void login()
    {
        CommandContainer cont;
        cont.set_cmd_id(1);
        Command_Login cmd;
        cmd.set_user_name("mod");
        cont.add_session_command()->MutableExtension(Command_Login::ext)->CopyFrom(cmd);
        handler.processCommandContainer(cont);
    }

    Response::ResponseCode sendPing()
    {
        CommandContainer cont;
        cont.set_cmd_id(2);
        cont.add_session_command()->MutableExtension(Command_Ping::ext);
        handler.processCommandContainer(cont);
        return handler.lastResponseCode;
    }

    Response::ResponseCode sendSessionCommand()
    {
        CommandContainer cont;
        cont.set_cmd_id(3);
        cont.add_session_command()->MutableExtension(Command_ListRooms::ext);
        handler.processCommandContainer(cont);
        return handler.lastResponseCode;
    }

    Response::ResponseCode sendRoomCommand()
    {
        CommandContainer cont;
        cont.set_cmd_id(4);
        cont.add_room_command()->MutableExtension(Command_RoomSay::ext)->set_message("hello");
        handler.processCommandContainer(cont);
        return handler.lastResponseCode;
    }

    Response::ResponseCode sendMixedSessionAndRoomCommand()
    {
        CommandContainer cont;
        cont.set_cmd_id(5);
        cont.add_session_command()->MutableExtension(Command_Ping::ext);
        cont.add_room_command()->MutableExtension(Command_RoomSay::ext)->set_message("hello");
        handler.processCommandContainer(cont);
        return handler.lastResponseCode;
    }

    Response::ResponseCode sendAccountPassword()
    {
        CommandContainer cont;
        cont.set_cmd_id(6);
        Command_AccountPassword cmd;
        cmd.set_old_password("temporary-password");
        cmd.set_new_password("replacement-password");
        cont.add_session_command()->MutableExtension(Command_AccountPassword::ext)->CopyFrom(cmd);
        handler.processCommandContainer(cont);
        return handler.lastResponseCode;
    }
};

TEST_F(ForcedPasswordChangeTest, LoginReportsTheRequiredChange)
{
    login();

    EXPECT_EQ(Response::RespPasswordChangeRequired, handler.lastResponseCode);
    ASSERT_NE(nullptr, handler.getUserInfo());
    EXPECT_EQ("mod", QString::fromStdString(handler.getUserInfo()->name()));
}

TEST_F(ForcedPasswordChangeTest, PendingSessionIsNotRegisteredAsAUser)
{
    login();
    // The session may change its password, but it must not take a user slot or appear in
    // the session table before the client reconnects with the new password.
    EXPECT_FALSE(handler.getUserInfo()->has_session_id());
    EXPECT_EQ(0, server.getUsersCount());
}

TEST_F(ForcedPasswordChangeTest, PendingModeratorIsNotReportedOnline)
{
    login();
    server.addClient(&handler);

    EXPECT_TRUE(server.getOnlineModeratorList().isEmpty());
}

TEST_F(ForcedPasswordChangeTest, OnlyPingAndPasswordChangeAreServed)
{
    login();

    EXPECT_EQ(Response::RespOk, sendPing());
    EXPECT_EQ(Response::RespFunctionNotAllowed, sendSessionCommand());
    EXPECT_EQ(Response::RespFunctionNotAllowed, sendRoomCommand());
    // Dispatch prefers game/room over session, so a container carrying both must not slip past
    // the restriction by carrying a session command.
    EXPECT_EQ(Response::RespFunctionNotAllowed, sendMixedSessionAndRoomCommand());
}

TEST_F(ForcedPasswordChangeTest, AccountPasswordReachesTheCommand)
{
    login();
    // The forced change is useless if the gate refuses the command it needs. ACCOUNT_PASSWORD is
    // served by the extended-command hook, so it must arrive there instead of being filtered.
    EXPECT_EQ(Response::RespOk, sendAccountPassword());
    EXPECT_EQ(SessionCommand::ACCOUNT_PASSWORD, handler.lastExtendedSessionCommand);
}

TEST_F(ForcedPasswordChangeTest, CompletedLoginIsStillReportedOnline)
{
    TestHandler loggedIn{&server, server.getDatabaseInterface()};
    ServerInfo_User user;
    user.set_name("online-mod");
    user.set_user_level(ServerInfo_User::IsUser | ServerInfo_User::IsRegistered | ServerInfo_User::IsModerator);
    user.set_session_id(7);
    loggedIn.setUserInfo(user);
    server.addClient(&loggedIn);

    const QList<QString> moderators = server.getOnlineModeratorList();
    ASSERT_EQ(1, moderators.size());
    EXPECT_EQ("online-mod", moderators.first());
}

} // namespace
