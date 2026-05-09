#ifndef COCKATRICE_GAME_FILTER_CONFIGS_H
#define COCKATRICE_GAME_FILTER_CONFIGS_H

#include <QTime>

/**
 * @brief The possible game filter configs.
 */
struct GameFilterConfigs
{
    static constexpr int DEFAULT_MAX_PLAYERS_MIN = 1;
    static constexpr int DEFAULT_MAX_PLAYERS_MAX = 99;

    bool hideBuddiesOnlyGames = false;
    bool hideIgnoredUserGames = false;
    bool hideFullGames = false;
    bool hideGamesThatStarted = false;
    bool hidePasswordProtectedGames = false;
    bool hideNotBuddyCreatedGames = false;
    bool hideOpenDecklistGames = false;
    QString gameNameFilter = "";
    QStringList creatorNameFilters = {};
    QSet<int> gameTypeFilter = {};
    int maxPlayersFilterMin = DEFAULT_MAX_PLAYERS_MIN;
    int maxPlayersFilterMax = DEFAULT_MAX_PLAYERS_MAX;
    QTime maxGameAge = {};
    bool showOnlyIfSpectatorsCanWatch = false;
    bool showSpectatorPasswordProtected = false;
    bool showOnlyIfSpectatorsCanChat = false;
    bool showOnlyIfSpectatorsCanSeeHands = false;

    bool operator==(const GameFilterConfigs &) const = default;

    /**
     * @brief Checks if this config has exactly the default values.
     *
     * @return Whether this config has the default values
     */
    bool isDefault() const;
};

#endif // COCKATRICE_GAME_FILTER_CONFIGS_H
