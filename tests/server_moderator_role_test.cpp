/** @file server_moderator_role_test.cpp
 *  @brief Tests for the moderator staff role authorization and dispatch.
 *  @ingroup Tests
 */

#include <gtest/gtest.h>
#include <libcockatrice/network/server/remote/server.h>
#include <libcockatrice/network/server/remote/server_protocolhandler.h>
#include <libcockatrice/protocol/pb/command_report_reopen.pb.h>
#include <libcockatrice/protocol/pb/commands.pb.h>
#include <libcockatrice/protocol/pb/moderator_commands.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_user.pb.h>
#include <libcockatrice/rng/rng_abstract.h>

// The server_remote library references the global RNG, which is normally
// defined by the servatrice/client executable main(). Provide a stub so the
// unit test can link against it.
RNG_Abstract *rng = nullptr;

namespace
{

class TestModeratorHandler : public Server_ProtocolHandler
{
public:
    explicit TestModeratorHandler(Server *_server) : Server_ProtocolHandler(_server, nullptr)
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

    // Buffer the last response code sent to the client so tests can assert on
    // the outcome of processCommandContainer().
    Response::ResponseCode lastResponseCode = Response::RespNothing;
    int dispatchCount = 0;

protected:
    void transmitProtocolItem(const ServerMessage &item) override
    {
        if (item.message_type() == ServerMessage::RESPONSE) {
            lastResponseCode = item.response().response_code();
        }
    }

    Response::ResponseCode
    processExtendedModeratorCommand(int cmdType, const ModeratorCommand &, ResponseContainer &) override
    {
        ++dispatchCount;
        // Fail closed for anything not explicitly handled.
        if (cmdType != ModeratorCommand::REPORT_REOPEN) {
            return Response::RespFunctionNotAllowed;
        }
        return Response::RespOk;
    }
};

class ModeratorRoleTest : public ::testing::Test
{
protected:
    Server server;
    TestModeratorHandler handler{&server};

    void setUserLevel(uint32_t level)
    {
        ServerInfo_User user;
        user.set_user_level(level);
        handler.setUserInfo(user);
    }
};

TEST_F(ModeratorRoleTest, RejectsWhenNotLoggedIn)
{
    CommandContainer cont;
    cont.add_moderator_command();
    handler.processCommandContainer(cont);
    EXPECT_EQ(handler.lastResponseCode, Response::RespLoginNeeded);
    EXPECT_EQ(handler.dispatchCount, 0);
}

TEST_F(ModeratorRoleTest, RejectsPlainUser)
{
    setUserLevel(ServerInfo_User::IsUser | ServerInfo_User::IsRegistered);

    CommandContainer cont;
    cont.add_moderator_command();
    handler.processCommandContainer(cont);
    EXPECT_EQ(handler.lastResponseCode, Response::RespLoginNeeded);
    EXPECT_EQ(handler.dispatchCount, 0);
}

TEST_F(ModeratorRoleTest, DispatchesToModeratorCommandForModerator)
{
    setUserLevel(ServerInfo_User::IsModerator);

    CommandContainer cont;
    ModeratorCommand *cmd = cont.add_moderator_command();
    cmd->MutableExtension(Command_ReportReopen::ext);
    handler.processCommandContainer(cont);
    EXPECT_EQ(handler.lastResponseCode, Response::RespOk);
    EXPECT_EQ(handler.dispatchCount, 1);
}

TEST_F(ModeratorRoleTest, DispatchesToModeratorCommandForAdmin)
{
    setUserLevel(ServerInfo_User::IsModerator | ServerInfo_User::IsAdmin);

    CommandContainer cont;
    ModeratorCommand *cmd = cont.add_moderator_command();
    cmd->MutableExtension(Command_ReportReopen::ext);
    handler.processCommandContainer(cont);
    EXPECT_EQ(handler.lastResponseCode, Response::RespOk);
    EXPECT_EQ(handler.dispatchCount, 1);
}

TEST_F(ModeratorRoleTest, RejectsDeveloperThatIsNotModerator)
{
    setUserLevel(ServerInfo_User::IsDeveloper);

    CommandContainer cont;
    cont.add_moderator_command();
    handler.processCommandContainer(cont);
    EXPECT_EQ(handler.lastResponseCode, Response::RespLoginNeeded);
    EXPECT_EQ(handler.dispatchCount, 0);
}

TEST_F(ModeratorRoleTest, FailClosedForUnknownModeratorCommand)
{
    setUserLevel(ServerInfo_User::IsModerator);

    CommandContainer cont;
    cont.add_moderator_command(); // no extension set -> getPbExtension() returns -1
    handler.processCommandContainer(cont);
    EXPECT_EQ(handler.lastResponseCode, Response::RespFunctionNotAllowed);
    EXPECT_EQ(handler.dispatchCount, 1);
}

} // namespace

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
