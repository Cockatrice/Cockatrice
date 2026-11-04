/** @file client_login_status_test.cpp
 *  @brief The login response state machine must keep the session a forced password change needs.
 *  @ingroup Tests
 */

#include "abstract_client.h"
#include "libcockatrice/interfaces/interface_network_settings_provider.h"
#include "libcockatrice/protocol/pb/response.pb.h"
#include "libcockatrice/protocol/pb/response_login.pb.h"
#include "remote_client.h"

#include <QCoreApplication>
#include <QMetaObject>
#include <QString>
#include <gtest/gtest.h>

namespace
{

class StubNetworkSettingsProvider : public INetworkSettingsProvider
{
public:
    QString getClientID() override
    {
        return {};
    }
    [[nodiscard]] int getTimeOut() const override
    {
        return 1000;
    }
    [[nodiscard]] int getKeepAlive() const override
    {
        return 60;
    }
    [[nodiscard]] bool getNotifyAboutUpdates() const override
    {
        return false;
    }
    void setKnownMissingFeatures(const QString &) override
    {
    }
    QString getKnownMissingFeatures() override
    {
        return {};
    }
};

/**
 * @brief Feeds a login response to RemoteClient without a socket.
 *
 * `loginResponse` is the slot the socket thread runs for the login command's answer, so
 * invoking it directly exercises the status transition that decides whether `readData()`
 * tears the connection down right after it.
 */
class ClientLoginStatusTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        client = new RemoteClient(nullptr, &provider);
    }

    void TearDown() override
    {
        delete client;
        client = nullptr;
    }

    void deliverLoginResponse(Response::ResponseCode code)
    {
        Response response;
        response.set_response_code(code);
        response.MutableExtension(Response_Login::ext);
        ASSERT_TRUE(
            QMetaObject::invokeMethod(client, "loginResponse", Qt::DirectConnection, Q_ARG(Response, response)));
    }

    StubNetworkSettingsProvider provider;
    RemoteClient *client = nullptr;
};

/**
 * @brief A required password change is not a failed login.
 *
 * The server keeps the session open and accepts only ping and Command_AccountPassword until
 * the password is replaced. StatusDisconnecting would make readData() close the socket right
 * after this slot returns - before the dialog that collects the new password (queued to the
 * UI thread) can send anything - and the pending command would never be answered because the
 * ping timer that times it out dies with the connection. StatusLoggedIn would start the
 * normal post-login flow for a session that cannot serve it.
 */
TEST_F(ClientLoginStatusTest, PasswordChangeRequiredKeepsTheSessionOpen)
{
    deliverLoginResponse(Response::RespPasswordChangeRequired);

    EXPECT_EQ(client->getStatus(), StatusPasswordChangeRequired);
    EXPECT_NE(client->getStatus(), StatusDisconnecting);
    EXPECT_NE(client->getStatus(), StatusLoggedIn);
}

/** @brief Every other login error still ends the connection, as it did before. */
TEST_F(ClientLoginStatusTest, OtherLoginErrorsDisconnect)
{
    deliverLoginResponse(Response::RespWrongPassword);

    EXPECT_EQ(client->getStatus(), StatusDisconnecting);
}

} // namespace

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
