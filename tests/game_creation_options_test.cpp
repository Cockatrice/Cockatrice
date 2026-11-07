#include "interface/widgets/game_hosting/game_creation_options.h"

#include <gtest/gtest.h>
#include <libcockatrice/protocol/pb/room_commands.pb.h>

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
