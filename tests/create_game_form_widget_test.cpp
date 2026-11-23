#include "interface/widgets/game_hosting/create_game_form_widget.h"

#include <QApplication>
#include <QMap>
#include <gtest/gtest.h>

namespace
{
const QMap<int, QString> gameTypes{{1, "Draft"}, {2, "Commander"}};

void expectSameOptions(const GameCreationOptions &actual, const GameCreationOptions &expected)
{
    EXPECT_EQ(actual.description, expected.description);
    EXPECT_EQ(actual.password, expected.password);
    EXPECT_EQ(actual.maxPlayers, expected.maxPlayers);
    EXPECT_EQ(actual.onlyBuddies, expected.onlyBuddies);
    EXPECT_EQ(actual.onlyRegistered, expected.onlyRegistered);
    EXPECT_EQ(actual.spectatorsAllowed, expected.spectatorsAllowed);
    EXPECT_EQ(actual.spectatorsNeedPassword, expected.spectatorsNeedPassword);
    EXPECT_EQ(actual.spectatorsCanTalk, expected.spectatorsCanTalk);
    EXPECT_EQ(actual.spectatorsSeeEverything, expected.spectatorsSeeEverything);
    EXPECT_EQ(actual.joinAsJudge, expected.joinAsJudge);
    EXPECT_EQ(actual.joinAsSpectator, expected.joinAsSpectator);
    EXPECT_EQ(actual.startingLifeTotal, expected.startingLifeTotal);
    EXPECT_EQ(actual.shareDecklistsOnLoad, expected.shareDecklistsOnLoad);
    EXPECT_EQ(actual.shufflePlayers, expected.shufflePlayers);
    EXPECT_EQ(actual.gameTypeIds, expected.gameTypeIds);
}
} // namespace

TEST(CreateGameFormWidget, RegisteredHostDefaultsToRegisteredOnly)
{
    const CreateGameFormWidget form(gameTypes, true, false);

    const GameCreationOptions options = form.options();

    EXPECT_TRUE(options.onlyRegistered);
    EXPECT_FALSE(options.onlyBuddies);
    EXPECT_EQ(options.maxPlayers, 2);
    EXPECT_TRUE(options.spectatorsAllowed);
    EXPECT_EQ(options.startingLifeTotal, 20);
    EXPECT_TRUE(options.gameTypeIds.isEmpty());
}

TEST(CreateGameFormWidget, UnregisteredHostDefaultsToOpenJoin)
{
    const CreateGameFormWidget form(gameTypes, false, false);

    const GameCreationOptions options = form.options();

    EXPECT_FALSE(options.onlyRegistered);
    EXPECT_FALSE(options.onlyBuddies);
}

TEST(CreateGameFormWidget, SetOptionsRoundTripsEveryField)
{
    CreateGameFormWidget form(gameTypes, true, true);

    GameCreationOptions expected;
    expected.description = "Some game";
    expected.password = "hunter2";
    expected.maxPlayers = 5;
    expected.onlyBuddies = true;
    expected.onlyRegistered = false;
    expected.spectatorsAllowed = false;
    expected.spectatorsNeedPassword = true;
    expected.spectatorsCanTalk = true;
    expected.spectatorsSeeEverything = true;
    expected.joinAsJudge = true;
    expected.joinAsSpectator = true;
    expected.startingLifeTotal = 40;
    expected.shareDecklistsOnLoad = true;
    expected.shufflePlayers = true;
    expected.gameTypeIds = {2};

    form.setOptions(expected);

    expectSameOptions(form.options(), expected);
}

TEST(CreateGameFormWidget, ResetRestoresDefaultsAndClearsTheGameType)
{
    CreateGameFormWidget form(gameTypes, true, false);
    GameCreationOptions configured;
    configured.description = "Configured";
    configured.maxPlayers = 8;
    configured.onlyBuddies = true;
    configured.joinAsJudge = true;
    configured.gameTypeIds = {2};
    form.setOptions(configured);

    form.resetToDefaults();

    const GameCreationOptions options = form.options();
    EXPECT_TRUE(options.description.isEmpty());
    EXPECT_EQ(options.maxPlayers, 2);
    EXPECT_FALSE(options.onlyBuddies);
    EXPECT_TRUE(options.onlyRegistered);
    EXPECT_TRUE(options.spectatorsAllowed);
    EXPECT_EQ(options.startingLifeTotal, 20);
    EXPECT_FALSE(options.joinAsJudge);
    EXPECT_TRUE(options.gameTypeIds.isEmpty());
}

TEST(CreateGameFormWidget, RememberSettingsFlagRoundTrips)
{
    CreateGameFormWidget form(gameTypes, true, false);

    EXPECT_FALSE(form.isRememberSettingsChecked());

    form.setRememberSettingsChecked(true);

    EXPECT_TRUE(form.isRememberSettingsChecked());
}

TEST(CreateGameFormWidget, SelectedGameTypesSettingStringUsesTheLegacyEncoding)
{
    CreateGameFormWidget form(gameTypes, true, false);
    GameCreationOptions configured;
    configured.gameTypeIds = {1};
    form.setOptions(configured);

    EXPECT_EQ(form.selectedGameTypesSettingString(), "Draft, ");
}

int main(int argc, char **argv)
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
