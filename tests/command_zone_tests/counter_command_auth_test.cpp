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

// evaluateDelCounter

TEST(EvaluateDelCounter, RejectsWhenGameNotStarted)
{
    Server_Counter counter = makeCounter(UserCounterId, 0);
    EXPECT_EQ(
        Server_Player::evaluateDelCounter(/*gameStarted=*/false, /*playerConceded=*/false, UserCounterId, &counter),
        Response::RespGameNotStarted);
}

TEST(EvaluateDelCounter, RejectsWhenPlayerConceded)
{
    Server_Counter counter = makeCounter(UserCounterId, 0);
    EXPECT_EQ(Server_Player::evaluateDelCounter(/*gameStarted=*/true, /*playerConceded=*/true, UserCounterId, &counter),
              Response::RespContextError);
}

TEST(EvaluateDelCounter, RejectsTaxCounters)
{
    Server_Counter commander = makeCounter(CounterIds::CommanderTax, 0);
    EXPECT_EQ(Server_Player::evaluateDelCounter(true, false, CounterIds::CommanderTax, &commander),
              Response::RespFunctionNotAllowed);

    Server_Counter partner = makeCounter(CounterIds::PartnerTax, 0);
    EXPECT_EQ(Server_Player::evaluateDelCounter(true, false, CounterIds::PartnerTax, &partner),
              Response::RespFunctionNotAllowed);
}

TEST(EvaluateDelCounter, RejectsMissingCounter)
{
    EXPECT_EQ(Server_Player::evaluateDelCounter(true, false, UserCounterId, nullptr), Response::RespNameNotFound);
}

TEST(EvaluateDelCounter, AllowsDeletingUserCounter)
{
    Server_Counter counter = makeCounter(UserCounterId, 7);
    EXPECT_EQ(Server_Player::evaluateDelCounter(true, false, UserCounterId, &counter), Response::RespOk);
}

TEST(EvaluateDelCounter, GameNotStartedTakesPrecedenceOverTaxGuard)
{
    Server_Counter commander = makeCounter(CounterIds::CommanderTax, 0);
    EXPECT_EQ(Server_Player::evaluateDelCounter(false, false, CounterIds::CommanderTax, &commander),
              Response::RespGameNotStarted);
}

// evaluateSetCounterActive

TEST(EvaluateSetCounterActive, RejectsWhenGameNotStarted)
{
    Server_Counter counter = makeCounter(CounterIds::CommanderTax, 0);
    EXPECT_EQ(Server_Player::evaluateSetCounterActive(/*gameStarted=*/false, /*playerConceded=*/false,
                                                      /*commandZoneEnabled=*/true, CounterIds::CommanderTax, &counter,
                                                      /*requestedActive=*/true),
              Response::RespGameNotStarted);
}

TEST(EvaluateSetCounterActive, RejectsWhenPlayerConceded)
{
    Server_Counter counter = makeCounter(CounterIds::CommanderTax, 0);
    EXPECT_EQ(Server_Player::evaluateSetCounterActive(true, /*playerConceded=*/true, true, CounterIds::CommanderTax,
                                                      &counter, true),
              Response::RespContextError);
}

TEST(EvaluateSetCounterActive, RejectsNonTaxCounter)
{
    Server_Counter counter = makeCounter(UserCounterId, 0);
    EXPECT_EQ(Server_Player::evaluateSetCounterActive(true, false, true, UserCounterId, &counter, true),
              Response::RespFunctionNotAllowed);
}

TEST(EvaluateSetCounterActive, RejectsWhenCommandZoneDisabled)
{
    Server_Counter counter = makeCounter(CounterIds::CommanderTax, 0);
    EXPECT_EQ(Server_Player::evaluateSetCounterActive(true, false, /*commandZoneEnabled=*/false,
                                                      CounterIds::CommanderTax, &counter, true),
              Response::RespContextError);
}

TEST(EvaluateSetCounterActive, RejectsMissingCounter)
{
    EXPECT_EQ(Server_Player::evaluateSetCounterActive(true, false, true, CounterIds::CommanderTax, nullptr, true),
              Response::RespNameNotFound);
}

TEST(EvaluateSetCounterActive, RejectsDisablingWhenTaxAccumulated)
{
    Server_Counter counter = makeCounter(CounterIds::CommanderTax, 3);
    EXPECT_EQ(Server_Player::evaluateSetCounterActive(true, false, true, CounterIds::CommanderTax, &counter,
                                                      /*requestedActive=*/false),
              Response::RespContextError);
}

TEST(EvaluateSetCounterActive, AllowsEnablingWithAccumulatedTax)
{
    Server_Counter counter = makeCounter(CounterIds::CommanderTax, 3);
    EXPECT_EQ(Server_Player::evaluateSetCounterActive(true, false, true, CounterIds::CommanderTax, &counter,
                                                      /*requestedActive=*/true),
              Response::RespOk);
}

TEST(EvaluateSetCounterActive, AllowsDisablingWhenCounterIsZero)
{
    Server_Counter counter = makeCounter(CounterIds::CommanderTax, 0);
    EXPECT_EQ(Server_Player::evaluateSetCounterActive(true, false, true, CounterIds::CommanderTax, &counter,
                                                      /*requestedActive=*/false),
              Response::RespOk);
}

TEST(EvaluateSetCounterActive, AllowsEnablingPartnerTax)
{
    Server_Counter counter = makeCounter(CounterIds::PartnerTax, 0);
    EXPECT_EQ(Server_Player::evaluateSetCounterActive(true, false, true, CounterIds::PartnerTax, &counter,
                                                      /*requestedActive=*/true),
              Response::RespOk);
}

TEST(EvaluateSetCounterActive, RejectsDisablingPartnerTaxWhenAccumulated)
{
    Server_Counter counter = makeCounter(CounterIds::PartnerTax, 2);
    EXPECT_EQ(Server_Player::evaluateSetCounterActive(true, false, true, CounterIds::PartnerTax, &counter,
                                                      /*requestedActive=*/false),
              Response::RespContextError);
}

// evaluateModifyCounter (shared by cmdIncCounter / cmdSetCounter)

TEST(EvaluateModifyCounter, RejectsWhenGameNotStarted)
{
    Server_Counter counter = makeCounter(UserCounterId, 0);
    EXPECT_EQ(Server_Player::evaluateModifyCounter(/*gameStarted=*/false, false, /*commandZoneEnabled=*/true,
                                                   UserCounterId, &counter),
              Response::RespGameNotStarted);
}

TEST(EvaluateModifyCounter, RejectsWhenPlayerConceded)
{
    Server_Counter counter = makeCounter(UserCounterId, 0);
    EXPECT_EQ(Server_Player::evaluateModifyCounter(true, /*playerConceded=*/true, true, UserCounterId, &counter),
              Response::RespContextError);
}

TEST(EvaluateModifyCounter, AllowsUserCounter)
{
    Server_Counter counter = makeCounter(UserCounterId, 0);
    EXPECT_EQ(Server_Player::evaluateModifyCounter(true, false, /*commandZoneEnabled=*/false, UserCounterId, &counter),
              Response::RespOk);
}

TEST(EvaluateModifyCounter, RejectsMissingCounter)
{
    EXPECT_EQ(Server_Player::evaluateModifyCounter(true, false, true, UserCounterId, nullptr),
              Response::RespNameNotFound);
}

TEST(EvaluateModifyCounter, RejectsTaxCounterWhenCommandZoneDisabled)
{
    Server_Counter counter = makeCounter(CounterIds::CommanderTax, 0);
    EXPECT_EQ(Server_Player::evaluateModifyCounter(true, false, /*commandZoneEnabled=*/false, CounterIds::CommanderTax,
                                                   &counter),
              Response::RespContextError);
}

TEST(EvaluateModifyCounter, RejectsInactiveTaxCounter)
{
    Server_Counter counter = makeCounter(CounterIds::PartnerTax, 0);
    (void)counter.setActive(false);
    EXPECT_EQ(Server_Player::evaluateModifyCounter(true, false, /*commandZoneEnabled=*/true, CounterIds::PartnerTax,
                                                   &counter),
              Response::RespContextError);
}

TEST(EvaluateModifyCounter, AllowsActiveTaxCounter)
{
    Server_Counter counter = makeCounter(CounterIds::CommanderTax, 0);
    EXPECT_EQ(Server_Player::evaluateModifyCounter(true, false, /*commandZoneEnabled=*/true, CounterIds::CommanderTax,
                                                   &counter),
              Response::RespOk);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
