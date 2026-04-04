#ifndef COCKATRICE_DISCORD_SOCIAL_CLIENT_H
#define COCKATRICE_DISCORD_SOCIAL_CLIENT_H
#include "discordpp.h"

#include <QObject>

class DiscordSocialClient : public QObject
{
public:
    DiscordSocialClient(QObject *parent = nullptr);
};

#endif // COCKATRICE_DISCORD_SOCIAL_CLIENT_H
