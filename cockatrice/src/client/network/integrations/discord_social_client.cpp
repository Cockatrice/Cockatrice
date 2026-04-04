#include "discord_social_client.h"

#define DISCORDPP_IMPLEMENTATION
#include "discordpp.h"

#include <QDebug>
#include <QTimer>

const uint64_t APPLICATION_ID = 1489873419875913832;

DiscordSocialClient::DiscordSocialClient(QObject *parent)
{
    Q_UNUSED(parent);

    auto client = std::make_shared<discordpp::Client>();
    client->AddLogCallback(
        [](auto message, auto severity) { qInfo() << "[" << EnumToString(severity) << "] " << message; },
        discordpp::LoggingSeverity::Info);

    client->SetStatusChangedCallback(
        [client](discordpp::Client::Status status, discordpp::Client::Error error, int32_t errorDetail) {
            qInfo() << "🔄 Status changed: " << discordpp::Client::StatusToString(status);

            if (status == discordpp::Client::Status::Ready) {
                qInfo() << "✅ Client is ready! You can now call SDK functions.\n";
                qInfo() << "👥 Friends Count: " << client->GetRelationships().size();
                // Configure rich presence details
                discordpp::Activity activity;
                activity.SetType(discordpp::ActivityTypes::Playing);
                activity.SetState("Coding features");
                activity.SetDetails("Working on integrating Discord");

                // Update rich presence
                client->UpdateRichPresence(activity, [](discordpp::ClientResult result) {
                    if (result.Successful()) {
                        qInfo() << "🎮 Rich Presence updated successfully!\n";
                    } else {
                        qInfo() << "❌ Rich Presence update failed";
                    }
                });
            } else if (error != discordpp::Client::Error::None) {
                qInfo() << "❌ Connection Error: " << discordpp::Client::ErrorToString(error)
                        << " - Details: " << errorDetail;
            }
        });

    // Generate OAuth2 code verifier for authentication
    auto codeVerifier = client->CreateAuthorizationCodeVerifier();

    // Set up authentication arguments
    discordpp::AuthorizationArgs args{};
    args.SetClientId(APPLICATION_ID);
    args.SetScopes(discordpp::Client::GetDefaultPresenceScopes());
    args.SetCodeChallenge(codeVerifier.Challenge());

    // Begin authentication process
    client->Authorize(args, [client, codeVerifier](auto result, auto code, auto redirectUri) {
        if (!result.Successful()) {
            qInfo() << "❌ Authentication Error: " << result.Error();
            return;
        } else {
            qInfo() << "✅ Authorization successful! Getting access token...\n";

            // Exchange auth code for access token
            client->GetToken(APPLICATION_ID, code, codeVerifier.Verifier(), redirectUri,
                             [client](discordpp::ClientResult result, std::string accessToken, std::string refreshToken,
                                      discordpp::AuthorizationTokenType tokenType, int32_t expiresIn,
                                      std::string scope) {
                                 Q_UNUSED(result);
                                 Q_UNUSED(expiresIn);
                                 Q_UNUSED(refreshToken);
                                 Q_UNUSED(tokenType);
                                 Q_UNUSED(scope);
                                 qInfo() << "🔓 Access token received! Establishing connection...\n";
                                 client->UpdateToken(discordpp::AuthorizationTokenType::Bearer, accessToken,
                                                     [client](discordpp::ClientResult result) {
                                                         if (result.Successful()) {
                                                             qInfo() << "🔑 Token updated, connecting to Discord...\n";
                                                             client->Connect();
                                                         }
                                                     });
                             });
        }
    });

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, []() { discordpp::RunCallbacks(); });

    timer->start(10);
}