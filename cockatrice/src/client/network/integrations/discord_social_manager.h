#ifndef COCKATRICE_DISCORD_SOCIAL_MANAGER_H
#define COCKATRICE_DISCORD_SOCIAL_MANAGER_H
#include "discord_social_client.h"

class DiscordSocialManager
{
public:
    DiscordSocialManager(const DiscordSocialManager &) = delete;

    DiscordSocialManager &operator=(const DiscordSocialManager &) = delete;

    /**
     * @brief Returns the singleton CardDatabase instance.
     * @return Pointer to the global CardDatabase.
     */
    static DiscordSocialClient *getInstance();

private:
    DiscordSocialManager() = default;

    ~DiscordSocialManager() = default;
};

#endif // COCKATRICE_DISCORD_SOCIAL_MANAGER_H
