/** @file server_game_join_test.cpp
 *  @brief Tests for the moderator/judge game-entry restriction override in Server_Game::checkJoin.
 *  @ingroup Tests
 */

#include "game/server_game.h"
#include "server.h"
#include "server_database_interface.h"
#include "server_room.h"

#include <gtest/gtest.h>
#include <libcockatrice/protocol/pb/serverinfo_user.pb.h>
#include <libcockatrice/rng/rng_abstract.h>

RNG_Abstract *rng = nullptr; // referenced by the server_remote library

namespace
{

class MockDatabaseInterface : public Server_DatabaseInterface
{
public:
    AuthenticationResult checkUserPassword(Server_ProtocolHandler *,
                                           const QString &,
                                           const QString &,
                                           const QString &,
                                           QString &,
                                           int &,
                                           bool) override
    {
        return NotLoggedIn;
    }
    int getNextReplayId() override
    {
        return 1;
    }
    int getNextGameId() override
    {
        return 1;
    }
    int getActiveUserCount(QString) override
    {
        return 0;
    }
    ServerInfo_User getUserData(const QString &, bool) override
    {
        return ServerInfo_User();
    }
};

class FakeServer : public Server
{
public:
    FakeServer()
    {
        setDatabaseInterface(new MockDatabaseInterface());
    }
};

class GameJoinOverrideTest : public ::testing::Test
{
protected:
    FakeServer server;
    Server_Room room{0, 0, "", "", "", "", false, "", {}, &server};
    ServerInfo_User creator;
    ServerInfo_User plainUser;
    ServerInfo_User unregisteredJudge;
    ServerInfo_User moderator;
    ServerInfo_User judge;
    Server_Game *game = nullptr;

    void SetUp() override
    {
        creator.set_name("creator");
        creator.set_user_level(ServerInfo_User::IsUser | ServerInfo_User::IsRegistered);
        plainUser.set_name("plain-user");
        plainUser.set_user_level(ServerInfo_User::IsUser | ServerInfo_User::IsRegistered);
        unregisteredJudge.set_name("unregistered-judge");
        unregisteredJudge.set_user_level(ServerInfo_User::IsUser | ServerInfo_User::IsJudge);
        moderator.set_name("moderator");
        moderator.set_user_level(ServerInfo_User::IsUser | ServerInfo_User::IsRegistered |
                                 ServerInfo_User::IsModerator);
        judge.set_name("judge");
        judge.set_user_level(ServerInfo_User::IsUser | ServerInfo_User::IsRegistered | ServerInfo_User::IsJudge);
    }

    void TearDown() override
    {
        delete game;
    }

    Server_Game *makeGame(bool passwordProtected, bool onlyRegistered, bool onlyBuddies, bool spectatorsAllowed)
    {
        GameConfig config{.creatorInfo = creator,
                          .gameId = 1,
                          .description = QString(),
                          .password = passwordProtected ? "secret" : QString(),
                          .maxPlayers = 2,
                          .gameTypes = QList<int>(),
                          .onlyBuddies = onlyBuddies,
                          .onlyRegistered = onlyRegistered,
                          .spectatorsAllowed = spectatorsAllowed,
                          .spectatorsNeedPassword = true,
                          .spectatorsCanTalk = false,
                          .spectatorsSeeEverything = false,
                          .startingLifeTotal = 20,
                          .shareDecklistsOnLoad = false};
        return new Server_Game(config, &room);
    }
};

TEST_F(GameJoinOverrideTest, StaffBypassPasswordRestriction)
{
    game = makeGame(true, false, false, true);

    // A plain user cannot override the password even with the override flag set.
    EXPECT_EQ(game->checkJoin(&plainUser, "wrong", false, true, false), Response::RespWrongPassword);
    // Moderators and judges may enter any game regardless of the password.
    EXPECT_EQ(game->checkJoin(&moderator, "wrong", false, true, false), Response::RespOk);
    EXPECT_EQ(game->checkJoin(&judge, "wrong", false, true, false), Response::RespOk);
    // Without the override flag judges are still subject to the password.
    EXPECT_EQ(game->checkJoin(&judge, "wrong", false, false, true), Response::RespWrongPassword);
    EXPECT_EQ(game->checkJoin(&judge, "secret", false, false, true), Response::RespOk);
}

TEST_F(GameJoinOverrideTest, StaffBypassRegisteredOnlyRestriction)
{
    game = makeGame(false, true, false, true);

    // Without the override flag the only-registered restriction still applies.
    EXPECT_EQ(game->checkJoin(&unregisteredJudge, QString(), false, false, false), Response::RespUserLevelTooLow);
    // An unregistered judge may enter when overriding restrictions.
    EXPECT_EQ(game->checkJoin(&unregisteredJudge, QString(), false, true, false), Response::RespOk);
}

TEST_F(GameJoinOverrideTest, StaffBypassBuddiesOnlyRestriction)
{
    game = makeGame(false, false, true, true);

    // A plain user who is not on the creator's buddy list gets rejected.
    EXPECT_EQ(game->checkJoin(&plainUser, QString(), false, true, false), Response::RespOnlyBuddies);
    // Moderators and judges bypass the buddies-only restriction.
    EXPECT_EQ(game->checkJoin(&moderator, QString(), false, true, false), Response::RespOk);
    EXPECT_EQ(game->checkJoin(&judge, QString(), false, true, false), Response::RespOk);
}

TEST_F(GameJoinOverrideTest, StaffBypassSpectatorsNotAllowedRestriction)
{
    game = makeGame(false, false, false, false);

    // A plain user cannot spectate when the game disallows spectators.
    EXPECT_EQ(game->checkJoin(&plainUser, QString(), true, false, false), Response::RespSpectatorsNotAllowed);
    // Moderators and judges may spectate any game regardless of the password
    // and the spectator restriction.
    EXPECT_EQ(game->checkJoin(&moderator, "wrong", true, true, false), Response::RespOk);
    EXPECT_EQ(game->checkJoin(&judge, "wrong", true, true, false), Response::RespOk);
}

TEST_F(GameJoinOverrideTest, JudgeOverrideDoesNotGrantJudgeJoinToPlainUser)
{
    game = makeGame(false, false, false, true);

    // joining with join_as_judge still requires the judge flag even when overriding.
    EXPECT_EQ(game->checkJoin(&plainUser, QString(), false, true, true), Response::RespUserLevelTooLow);
    EXPECT_EQ(game->checkJoin(&judge, QString(), false, true, true), Response::RespOk);
}

} // namespace

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}