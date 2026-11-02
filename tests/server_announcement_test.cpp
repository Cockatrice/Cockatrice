#include "announcement_utils.h"

#include <gtest/gtest.h>
#include <libcockatrice/network/server/remote/server.h>
#include <libcockatrice/network/server/remote/server_protocolhandler.h>
#include <libcockatrice/protocol/pb/admin_commands.pb.h>
#include <libcockatrice/protocol/pb/commands.pb.h>
#include <libcockatrice/protocol/pb/response.pb.h>
#include <libcockatrice/protocol/pb/server_message.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_user.pb.h>
#include <libcockatrice/rng/rng_abstract.h>
#include <string>

RNG_Abstract *rng = nullptr;

namespace
{
TEST(ServerAnnouncement, RejectsEmptyOversizedAndInvalidUtf8)
{
    QString result;
    EXPECT_FALSE(normalizeServerAnnouncement("", result));
    EXPECT_FALSE(normalizeServerAnnouncement(" \t\n", result));
    EXPECT_FALSE(normalizeServerAnnouncement(std::string(MAX_TEXT_LENGTH + 1, 'x'), result));
    EXPECT_FALSE(normalizeServerAnnouncement(std::string("\xff", 1), result));
    EXPECT_TRUE(normalizeServerAnnouncement(std::string(MAX_TEXT_LENGTH, 'x'), result));
    EXPECT_EQ(result.size(), MAX_TEXT_LENGTH);
    EXPECT_TRUE(normalizeServerAnnouncement("  Hello   world  ", result));
    EXPECT_EQ(result, QStringLiteral("Hello world"));
}

TEST(ServerAnnouncement, ExistingCustomEventCarriesMessageAcrossWire)
{
    const Event_NotifyUser original = makeServerAnnouncementEvent(QString::fromUtf8("Hello café"));
    SessionEvent envelope;
    envelope.MutableExtension(Event_NotifyUser::ext)->CopyFrom(original);
    SessionEvent received;
    ASSERT_TRUE(received.ParseFromString(envelope.SerializeAsString()));
    ASSERT_TRUE(received.HasExtension(Event_NotifyUser::ext));
    const auto &event = received.GetExtension(Event_NotifyUser::ext);
    EXPECT_EQ(event.type(), Event_NotifyUser::CUSTOM);
    EXPECT_EQ(event.custom_title(), "Server announcement");
    EXPECT_EQ(event.custom_content(), "Hello café");
}

class AdminGateHandler : public Server_ProtocolHandler
{
public:
    explicit AdminGateHandler(Server *server) : Server_ProtocolHandler(server, nullptr)
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
    Response::ResponseCode response = Response::RespNothing;
    int dispatched = 0;

protected:
    void transmitProtocolItem(const ServerMessage &item) override
    {
        if (item.message_type() == ServerMessage::RESPONSE) {
            response = item.response().response_code();
        }
    }
    Response::ResponseCode
    processExtendedAdminCommand(int type, const AdminCommand &command, ResponseContainer &) override
    {
        ++dispatched;
        if (type != AdminCommand::ANNOUNCE) {
            return Response::RespFunctionNotAllowed;
        }
        QString message;
        const auto &input = command.GetExtension(Command_Announce::ext);
        return input.has_message() && normalizeServerAnnouncement(input.message(), message) ? Response::RespOk
                                                                                            : Response::RespInvalidData;
    }
};

TEST(ServerAnnouncement, ExistingAdminGateRejectsGuestsAndModerators)
{
    Server server;
    AdminGateHandler handler(&server);
    CommandContainer command;
    command.add_admin_command()->MutableExtension(Command_Announce::ext)->set_message("Maintenance");
    handler.processCommandContainer(command);
    EXPECT_EQ(handler.response, Response::RespLoginNeeded);
    EXPECT_EQ(handler.dispatched, 0);

    ServerInfo_User user;
    user.set_user_level(ServerInfo_User::IsModerator);
    handler.setUserInfo(user);
    handler.processCommandContainer(command);
    EXPECT_EQ(handler.response, Response::RespLoginNeeded);
    EXPECT_EQ(handler.dispatched, 0);

    user.set_user_level(ServerInfo_User::IsAdmin);
    handler.setUserInfo(user);
    handler.processCommandContainer(command);
    EXPECT_EQ(handler.response, Response::RespOk);
    EXPECT_EQ(handler.dispatched, 1);
}
} // namespace
