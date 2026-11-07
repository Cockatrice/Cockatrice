#include "interface/widgets/game_hosting/game_creation_options.h"

#include <gtest/gtest.h>
#include <libcockatrice/interfaces/interface_game_settings_provider.h>
#include <libcockatrice/protocol/pb/room_commands.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_game.pb.h>

namespace
{
class FakeGameSettings : public IGameSettingsProvider
{
public:
    [[nodiscard]] QString getGameDescription() const override
    {
        return description;
    }
    [[nodiscard]] int getMaxPlayers() const override
    {
        return maxPlayers;
    }
    [[nodiscard]] QString getGameTypes() const override
    {
        return gameTypes;
    }
    [[nodiscard]] bool getOnlyBuddies() const override
    {
        return onlyBuddies;
    }
    [[nodiscard]] bool getOnlyRegistered() const override
    {
        return onlyRegistered;
    }
    [[nodiscard]] bool getSpectatorsAllowed() const override
    {
        return spectatorsAllowed;
    }
    [[nodiscard]] bool getSpectatorsNeedPassword() const override
    {
        return spectatorsNeedPassword;
    }
    [[nodiscard]] bool getSpectatorsCanTalk() const override
    {
        return spectatorsCanTalk;
    }
    [[nodiscard]] bool getSpectatorsCanSeeEverything() const override
    {
        return spectatorsCanSeeEverything;
    }
    [[nodiscard]] bool getCreateGameAsSpectator() const override
    {
        return createGameAsSpectator;
    }
    [[nodiscard]] int getDefaultStartingLifeTotal() const override
    {
        return defaultStartingLifeTotal;
    }
    [[nodiscard]] bool getShareDecklistsOnLoad() const override
    {
        return shareDecklistsOnLoad;
    }
    [[nodiscard]] bool getShufflePlayers() const override
    {
        return shufflePlayers;
    }
    [[nodiscard]] bool getRememberGameSettings() const override
    {
        return false;
    }
    [[nodiscard]] bool getLocalGameRememberSettings() const override
    {
        return false;
    }
    [[nodiscard]] int getLocalGameMaxPlayers() const override
    {
        return 0;
    }
    [[nodiscard]] int getLocalGameStartingLifeTotal() const override
    {
        return 0;
    }

    QString description;
    int maxPlayers = 2;
    QString gameTypes;
    bool onlyBuddies = false;
    bool onlyRegistered = false;
    bool spectatorsAllowed = true;
    bool spectatorsNeedPassword = false;
    bool spectatorsCanTalk = false;
    bool spectatorsCanSeeEverything = false;
    bool createGameAsSpectator = false;
    int defaultStartingLifeTotal = 20;
    bool shareDecklistsOnLoad = false;
    bool shufflePlayers = false;
};
} // namespace

TEST(GameCreationOptions, DefaultsMatchTheDialog)
{
    const GameCreationOptions options;

    EXPECT_TRUE(options.description.isEmpty());
    EXPECT_TRUE(options.password.isEmpty());
    EXPECT_EQ(options.maxPlayers, 2);
    EXPECT_FALSE(options.onlyBuddies);
    EXPECT_FALSE(options.onlyRegistered);
    EXPECT_TRUE(options.spectatorsAllowed);
    EXPECT_FALSE(options.spectatorsNeedPassword);
    EXPECT_FALSE(options.spectatorsCanTalk);
    EXPECT_FALSE(options.spectatorsSeeEverything);
    EXPECT_FALSE(options.joinAsJudge);
    EXPECT_FALSE(options.joinAsSpectator);
    EXPECT_EQ(options.startingLifeTotal, 20);
    EXPECT_FALSE(options.shareDecklistsOnLoad);
    EXPECT_FALSE(options.shufflePlayers);
    EXPECT_TRUE(options.gameTypeIds.isEmpty());
}

TEST(GameCreationOptions, EveryFieldMapsToTheCommand)
{
    GameCreationOptions options;
    options.description = "Some game";
    options.password = "hunter2";
    options.maxPlayers = 4;
    options.onlyBuddies = true;
    options.onlyRegistered = true;
    options.spectatorsAllowed = false;
    options.spectatorsNeedPassword = true;
    options.spectatorsCanTalk = true;
    options.spectatorsSeeEverything = true;
    options.joinAsJudge = true;
    options.joinAsSpectator = true;
    options.startingLifeTotal = 40;
    options.shareDecklistsOnLoad = true;
    options.shufflePlayers = true;

    const Command_CreateGame cmd = options.toCreateGameCommand();

    EXPECT_EQ(cmd.description(), "Some game");
    EXPECT_EQ(cmd.password(), "hunter2");
    EXPECT_EQ(cmd.max_players(), 4);
    EXPECT_TRUE(cmd.only_buddies());
    EXPECT_TRUE(cmd.only_registered());
    EXPECT_FALSE(cmd.spectators_allowed());
    EXPECT_TRUE(cmd.spectators_need_password());
    EXPECT_TRUE(cmd.spectators_can_talk());
    EXPECT_TRUE(cmd.spectators_see_everything());
    EXPECT_TRUE(cmd.join_as_judge());
    EXPECT_TRUE(cmd.join_as_spectator());
    EXPECT_EQ(cmd.starting_life_total(), 40);
    EXPECT_TRUE(cmd.share_decklists_on_load());
    EXPECT_TRUE(cmd.shuffle_players());
}

TEST(GameCreationOptions, GameTypeIdsKeepTheirOrder)
{
    GameCreationOptions options;
    options.gameTypeIds = {7, 2, 5};

    const Command_CreateGame cmd = options.toCreateGameCommand();

    ASSERT_EQ(cmd.game_type_ids_size(), 3);
    EXPECT_EQ(cmd.game_type_ids(0), 7);
    EXPECT_EQ(cmd.game_type_ids(1), 2);
    EXPECT_EQ(cmd.game_type_ids(2), 5);
}

TEST(GameCreationOptions, EmptyGameTypeIdsProduceAnEmptyRepeatedField)
{
    GameCreationOptions options;

    const Command_CreateGame cmd = options.toCreateGameCommand();

    EXPECT_EQ(cmd.game_type_ids_size(), 0);
}

TEST(GameCreationOptions, DescriptionWhitespaceIsSimplified)
{
    GameCreationOptions options;
    options.description = "  a   b  ";

    const Command_CreateGame cmd = options.toCreateGameCommand();

    EXPECT_EQ(cmd.description(), "a b");
}

TEST(GameCreationOptions, DefaultsForRegisteredHostEnforceRegistration)
{
    const GameCreationOptions registered = GameCreationOptions::defaultsFor(true);

    EXPECT_TRUE(registered.onlyRegistered);
    EXPECT_FALSE(registered.onlyBuddies);
    EXPECT_EQ(registered.maxPlayers, 2);
    EXPECT_TRUE(registered.spectatorsAllowed);
    EXPECT_EQ(registered.startingLifeTotal, 20);
    EXPECT_TRUE(registered.gameTypeIds.isEmpty());
}

TEST(GameCreationOptions, DefaultsForUnregisteredHostAllowOpenJoining)
{
    const GameCreationOptions unregistered = GameCreationOptions::defaultsFor(false);

    EXPECT_FALSE(unregistered.onlyRegistered);
    EXPECT_FALSE(unregistered.onlyBuddies);
}

TEST(GameCreationOptions, FromSettingsCopiesEveryRememberedField)
{
    FakeGameSettings settings;
    settings.description = "Remembered game";
    settings.maxPlayers = 6;
    settings.onlyBuddies = true;
    settings.onlyRegistered = true;
    settings.spectatorsAllowed = false;
    settings.spectatorsNeedPassword = true;
    settings.spectatorsCanTalk = true;
    settings.spectatorsCanSeeEverything = true;
    settings.createGameAsSpectator = true;
    settings.defaultStartingLifeTotal = 40;
    settings.shareDecklistsOnLoad = true;
    settings.shufflePlayers = true;

    const GameCreationOptions options = GameCreationOptions::fromSettings(settings, {}, true);

    EXPECT_EQ(options.description, "Remembered game");
    EXPECT_EQ(options.maxPlayers, 6);
    EXPECT_TRUE(options.onlyBuddies);
    EXPECT_TRUE(options.onlyRegistered);
    EXPECT_FALSE(options.spectatorsAllowed);
    EXPECT_TRUE(options.spectatorsNeedPassword);
    EXPECT_TRUE(options.spectatorsCanTalk);
    EXPECT_TRUE(options.spectatorsSeeEverything);
    EXPECT_TRUE(options.joinAsSpectator);
    EXPECT_EQ(options.startingLifeTotal, 40);
    EXPECT_TRUE(options.shareDecklistsOnLoad);
    EXPECT_TRUE(options.shufflePlayers);
}

TEST(GameCreationOptions, FromSettingsDropsJoinRestrictionsForUnregisteredHosts)
{
    FakeGameSettings settings;
    settings.onlyBuddies = true;
    settings.onlyRegistered = true;

    const GameCreationOptions options = GameCreationOptions::fromSettings(settings, {}, false);

    EXPECT_FALSE(options.onlyBuddies);
    EXPECT_FALSE(options.onlyRegistered);
}

TEST(GameCreationOptions, FromSettingsMatchesGameTypesByTheLegacyNameList)
{
    FakeGameSettings settings;
    settings.gameTypes = "Draft, Commander, ";
    const QMap<int, QString> gameTypes{{1, "Draft"}, {2, "Commander"}, {3, "Standard"}};

    const GameCreationOptions options = GameCreationOptions::fromSettings(settings, gameTypes, true);

    EXPECT_EQ(options.gameTypeIds, QList<int>({1, 2}));
}

TEST(GameCreationOptions, FromSettingsDoesNotMatchGameTypesWithoutTheLegacySeparator)
{
    FakeGameSettings settings;
    settings.gameTypes = "Draft Commander";
    const QMap<int, QString> gameTypes{{1, "Draft"}, {2, "Commander"}};

    const GameCreationOptions options = GameCreationOptions::fromSettings(settings, gameTypes, true);

    EXPECT_TRUE(options.gameTypeIds.isEmpty());
}

TEST(GameCreationOptions, FromGameInfoCopiesTheVisibleFields)
{
    ServerInfo_Game gameInfo;
    gameInfo.set_description("Some game");
    gameInfo.set_max_players(4);
    gameInfo.set_only_buddies(true);
    gameInfo.set_only_registered(true);
    gameInfo.set_spectators_allowed(false);
    gameInfo.set_spectators_need_password(true);
    gameInfo.set_spectators_can_chat(true);
    gameInfo.set_spectators_omniscient(true);
    gameInfo.set_share_decklists_on_load(true);
    gameInfo.set_shuffle_players(true);
    gameInfo.add_game_types(2);

    const QMap<int, QString> gameTypes{{1, "Draft"}, {2, "Commander"}};
    const GameCreationOptions options = GameCreationOptions::fromGameInfo(gameInfo, gameTypes);

    EXPECT_EQ(options.description, "Some game");
    EXPECT_EQ(options.maxPlayers, 4);
    EXPECT_TRUE(options.onlyBuddies);
    EXPECT_TRUE(options.onlyRegistered);
    EXPECT_FALSE(options.spectatorsAllowed);
    EXPECT_TRUE(options.spectatorsNeedPassword);
    EXPECT_TRUE(options.spectatorsCanTalk);
    EXPECT_TRUE(options.spectatorsSeeEverything);
    EXPECT_TRUE(options.shareDecklistsOnLoad);
    EXPECT_TRUE(options.shufflePlayers);
    EXPECT_EQ(options.gameTypeIds, QList<int>({2}));
}

TEST(GameCreationOptions, FromGameInfoLeavesUnshownFieldsAtDefaults)
{
    ServerInfo_Game gameInfo;
    gameInfo.set_max_players(3);

    const GameCreationOptions options = GameCreationOptions::fromGameInfo(gameInfo, {});

    EXPECT_TRUE(options.password.isEmpty());
    EXPECT_EQ(options.startingLifeTotal, 20);
    EXPECT_FALSE(options.joinAsJudge);
    EXPECT_FALSE(options.joinAsSpectator);
}

TEST(GameCreationOptions, FromGameInfoIgnoresUnknownGameTypeIds)
{
    ServerInfo_Game gameInfo;
    gameInfo.add_game_types(7);
    gameInfo.add_game_types(9);

    const QMap<int, QString> gameTypes{{1, "Draft"}, {7, "Commander"}};
    const GameCreationOptions options = GameCreationOptions::fromGameInfo(gameInfo, gameTypes);

    EXPECT_EQ(options.gameTypeIds, QList<int>({7}));
}
