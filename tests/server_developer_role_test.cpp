/** @file server_developer_role_test.cpp
 *  @brief Tests for the developer staff role authorization and dispatch.
 *  @ingroup Tests
 */

#include <gtest/gtest.h>
#include <libcockatrice/network/server/remote/server.h>
#include <libcockatrice/network/server/remote/server_protocolhandler.h>
#include <libcockatrice/protocol/pb/command_get_server_stats.pb.h>
#include <libcockatrice/protocol/pb/commands.pb.h>
#include <libcockatrice/protocol/pb/developer_commands.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_user.pb.h>
#include <libcockatrice/rng/rng_abstract.h>

// The server_remote library references the global RNG, which is normally
// defined by the servatrice/client executable main(). Provide a stub so the
// unit test can link against it.
RNG_Abstract *rng = nullptr;

namespace
{

class TestDeveloperHandler : public Server_ProtocolHandler
{
public:
    explicit TestDeveloperHandler(Server *_server) : Server_ProtocolHandler(_server, nullptr)
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
    processExtendedDeveloperCommand(int cmdType, const DeveloperCommand &, ResponseContainer &) override
    {
        ++dispatchCount;
        // Fail closed for anything not explicitly handled.
        if (cmdType != DeveloperCommand::GET_SERVER_STATS) {
            return Response::RespFunctionNotAllowed;
        }
        return Response::RespOk;
    }
};

class DeveloperRoleTest : public ::testing::Test
{
protected:
    Server server;
    TestDeveloperHandler handler{&server};

    void setUserLevel(uint32_t level)
    {
        ServerInfo_User user;
        user.set_user_level(level);
        handler.setUserInfo(user);
    }
};

TEST_F(DeveloperRoleTest, RejectsWhenNotLoggedIn)
{
    CommandContainer cont;
    cont.add_developer_command();
    handler.processCommandContainer(cont);
    EXPECT_EQ(handler.lastResponseCode, Response::RespLoginNeeded);
    EXPECT_EQ(handler.dispatchCount, 0);
}

TEST_F(DeveloperRoleTest, RejectsPlainUser)
{
    setUserLevel(ServerInfo_User::IsUser | ServerInfo_User::IsRegistered);

    CommandContainer cont;
    cont.add_developer_command();
    handler.processCommandContainer(cont);
    EXPECT_EQ(handler.lastResponseCode, Response::RespLoginNeeded);
    EXPECT_EQ(handler.dispatchCount, 0);
}

TEST_F(DeveloperRoleTest, RejectsModeratorThatIsNotDeveloper)
{
    setUserLevel(ServerInfo_User::IsModerator);

    CommandContainer cont;
    cont.add_developer_command();
    handler.processCommandContainer(cont);
    EXPECT_EQ(handler.lastResponseCode, Response::RespLoginNeeded);
}

TEST_F(DeveloperRoleTest, DispatchesToDeveloperCommandForDeveloper)
{
    setUserLevel(ServerInfo_User::IsDeveloper);

    CommandContainer cont;
    DeveloperCommand *cmd = cont.add_developer_command();
    cmd->MutableExtension(Command_GetServerStats::ext);
    handler.processCommandContainer(cont);
    EXPECT_EQ(handler.lastResponseCode, Response::RespOk);
    EXPECT_EQ(handler.dispatchCount, 1);
}

TEST_F(DeveloperRoleTest, FailClosedForUnknownDeveloperCommand)
{
    setUserLevel(ServerInfo_User::IsDeveloper);

    CommandContainer cont;
    cont.add_developer_command(); // no extension set -> getPbExtension() returns -1
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
