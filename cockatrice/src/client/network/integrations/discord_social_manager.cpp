#include "discord_social_manager.h"

DiscordSocialClient *DiscordSocialManager::getInstance()
{
    static DiscordSocialClient instance(nullptr);
    return &instance;
}