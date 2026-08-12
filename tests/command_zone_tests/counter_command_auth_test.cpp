/** @file counter_command_auth_test.cpp
 *  @brief Tests for Server_Player counter-command authorization helpers.
 *  @ingroup Tests
 */

#include "game/server_counter.h"
#include "game/server_player.h"

#include <gtest/gtest.h>
#include <libcockatrice/protocol/pb/color.pb.h>
#include <libcockatrice/protocol/pb/response.pb.h>
#include <libcockatrice/rng/rng_abstract.h>
#include <libcockatrice/utility/counter_ids.h>

RNG_Abstract *rng = nullptr; // required by linked server code

namespace
{
constexpr int UserCounterId = CounterIds::FirstUserId;

Server_Counter makeCounter(int id, int count)
{
    return Server_Counter(id, "c", color(), 20, count);
}
} // namespace

TEST(EvaluateDelCounter, RejectsWhenGameNotStarted)
{
    Server_Counter counter = makeCounter(UserCounterId, 0);
    EXPECT_EQ(Server_Player::evaluateDelCounter(/*gameStarted=*/false, /*playerConceded=*/false, &counter),
              Response::RespGameNotStarted);
}

TEST(EvaluateDelCounter, RejectsWhenPlayerConceded)
{
    Server_Counter counter = makeCounter(UserCounterId, 0);
    EXPECT_EQ(Server_Player::evaluateDelCounter(/*gameStarted=*/true, /*playerConceded=*/true, &counter),
              Response::RespContextError);
}

TEST(EvaluateDelCounter, RejectsMissingCounter)
{
    EXPECT_EQ(Server_Player::evaluateDelCounter(true, false, nullptr), Response::RespNameNotFound);
}

TEST(EvaluateDelCounter, AllowsDeletingUserCounter)
{
    Server_Counter counter = makeCounter(UserCounterId, 7);
    EXPECT_EQ(Server_Player::evaluateDelCounter(true, false, &counter), Response::RespOk);
}

TEST(EvaluateCreateCounter, RejectsWhenGameNotStarted)
{
    EXPECT_EQ(Server_Player::evaluateCreateCounter(/*gameStarted=*/false, /*playerConceded=*/false, "test"),
              Response::RespGameNotStarted);
}

TEST(EvaluateCreateCounter, RejectsWhenPlayerConceded)
{
    EXPECT_EQ(Server_Player::evaluateCreateCounter(true, /*playerConceded=*/true, "test"), Response::RespContextError);
}

TEST(EvaluateCreateCounter, AllowsUserCounterName)
{
    EXPECT_EQ(Server_Player::evaluateCreateCounter(true, false, "poison"), Response::RespOk);
}

TEST(EvaluateModifyCounter, RejectsWhenGameNotStarted)
{
    Server_Counter counter = makeCounter(UserCounterId, 0);
    EXPECT_EQ(Server_Player::evaluateModifyCounter(/*gameStarted=*/false, /*playerConceded=*/false, &counter),
              Response::RespGameNotStarted);
}

TEST(EvaluateModifyCounter, RejectsWhenPlayerConceded)
{
    Server_Counter counter = makeCounter(UserCounterId, 0);
    EXPECT_EQ(Server_Player::evaluateModifyCounter(true, /*playerConceded=*/true, &counter),
              Response::RespContextError);
}

TEST(EvaluateModifyCounter, RejectsMissingCounter)
{
    EXPECT_EQ(Server_Player::evaluateModifyCounter(true, false, nullptr), Response::RespNameNotFound);
}

TEST(EvaluateModifyCounter, AllowsUserCounter)
{
    Server_Counter counter = makeCounter(UserCounterId, 5);
    EXPECT_EQ(Server_Player::evaluateModifyCounter(true, false, &counter), Response::RespOk);
}

// Cast count evaluation tests

TEST(EvaluateCreateCastCount, RejectsWhenGameNotStarted)
{
    EXPECT_EQ(Server_Player::evaluateCreateCastCount(/*gameStarted=*/false, /*playerConceded=*/false,
                                                     /*commandZoneEnabled=*/true, 1, /*exists=*/false,
                                                     /*predecessorExists=*/true),
              Response::RespGameNotStarted);
}

TEST(EvaluateCreateCastCount, RejectsWhenPlayerConceded)
{
    EXPECT_EQ(Server_Player::evaluateCreateCastCount(true, /*playerConceded=*/true, true, 1, false, true),
              Response::RespContextError);
}

TEST(EvaluateCreateCastCount, RejectsWhenCommandZoneDisabled)
{
    EXPECT_EQ(Server_Player::evaluateCreateCastCount(true, false, /*commandZoneEnabled=*/false, 1, false, true),
              Response::RespContextError);
}

TEST(EvaluateCreateCastCount, RejectsInvalidIndex)
{
    EXPECT_EQ(Server_Player::evaluateCreateCastCount(true, false, true, 0, false, false), Response::RespContextError);
    EXPECT_EQ(Server_Player::evaluateCreateCastCount(true, false, true, 6, false, false), Response::RespContextError);
}

TEST(EvaluateCreateCastCount, RejectsExistingIndex)
{
    EXPECT_EQ(Server_Player::evaluateCreateCastCount(true, false, true, 1, /*exists=*/true, true),
              Response::RespContextError);
}

TEST(EvaluateCreateCastCount, RejectsGapInSequence)
{
    EXPECT_EQ(
        Server_Player::evaluateCreateCastCount(true, false, true, 3, /*exists=*/false, /*predecessorExists=*/false),
        Response::RespContextError);
}

TEST(EvaluateCreateCastCount, AllowsNextInSequence)
{
    EXPECT_EQ(
        Server_Player::evaluateCreateCastCount(true, false, true, 2, /*exists=*/false, /*predecessorExists=*/true),
        Response::RespOk);
}

TEST(EvaluateDeleteCastCount, RejectsWhenGameNotStarted)
{
    EXPECT_EQ(Server_Player::evaluateDeleteCastCount(/*gameStarted=*/false, /*playerConceded=*/false,
                                                     /*commandZoneEnabled=*/true, 1, /*exists=*/true, /*value=*/0,
                                                     /*successorExists=*/false),
              Response::RespGameNotStarted);
}

TEST(EvaluateDeleteCastCount, RejectsWhenPlayerConceded)
{
    EXPECT_EQ(Server_Player::evaluateDeleteCastCount(true, /*playerConceded=*/true, true, 1, true, 0, false),
              Response::RespContextError);
}

TEST(EvaluateDeleteCastCount, RejectsWhenCommandZoneDisabled)
{
    EXPECT_EQ(Server_Player::evaluateDeleteCastCount(true, false, /*commandZoneEnabled=*/false, 1, true, 0, false),
              Response::RespContextError);
}

TEST(EvaluateDeleteCastCount, RejectsMissingIndex)
{
    EXPECT_EQ(Server_Player::evaluateDeleteCastCount(true, false, true, 1, /*exists=*/false, 0, false),
              Response::RespNameNotFound);
}

TEST(EvaluateDeleteCastCount, RejectsNonZeroValue)
{
    EXPECT_EQ(Server_Player::evaluateDeleteCastCount(true, false, true, 1, true, /*value=*/5, false),
              Response::RespContextError);
}

TEST(EvaluateDeleteCastCount, RejectsNonHighestIndex)
{
    EXPECT_EQ(Server_Player::evaluateDeleteCastCount(true, false, true, 1, true, 0, /*successorExists=*/true),
              Response::RespContextError);
}

TEST(EvaluateDeleteCastCount, AllowsHighestWithZeroValue)
{
    EXPECT_EQ(Server_Player::evaluateDeleteCastCount(true, false, true, 2, /*exists=*/true, /*value=*/0,
                                                     /*successorExists=*/false),
              Response::RespOk);
}

TEST(EvaluateModifyCastCount, RejectsWhenGameNotStarted)
{
    EXPECT_EQ(Server_Player::evaluateModifyCastCount(/*gameStarted=*/false, /*playerConceded=*/false,
                                                     /*commandZoneEnabled=*/true, 1, /*exists=*/true),
              Response::RespGameNotStarted);
}

TEST(EvaluateModifyCastCount, RejectsWhenPlayerConceded)
{
    EXPECT_EQ(Server_Player::evaluateModifyCastCount(true, /*playerConceded=*/true, true, 1, true),
              Response::RespContextError);
}

TEST(EvaluateModifyCastCount, RejectsWhenCommandZoneDisabled)
{
    EXPECT_EQ(Server_Player::evaluateModifyCastCount(true, false, /*commandZoneEnabled=*/false, 1, true),
              Response::RespContextError);
}

TEST(EvaluateModifyCastCount, RejectsMissingIndex)
{
    EXPECT_EQ(Server_Player::evaluateModifyCastCount(true, false, true, 1, /*exists=*/false),
              Response::RespNameNotFound);
}

TEST(EvaluateModifyCastCount, AllowsExistingIndex)
{
    EXPECT_EQ(Server_Player::evaluateModifyCastCount(true, false, true, 1, /*exists=*/true), Response::RespOk);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
