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

Server_Counter makeCounter(int id, int count, bool active = true)
{
    Server_Counter c(id, "c", color(), 20, count);
    (void)c.setActive(active);
    return c;
}
} // namespace

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
    Server_Counter tax1 = makeCounter(CounterIds::TaxCounter1, 0);
    EXPECT_EQ(Server_Player::evaluateDelCounter(true, false, CounterIds::TaxCounter1, &tax1),
              Response::RespFunctionNotAllowed);

    Server_Counter tax2 = makeCounter(CounterIds::TaxCounter2, 0);
    EXPECT_EQ(Server_Player::evaluateDelCounter(true, false, CounterIds::TaxCounter2, &tax2),
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
    Server_Counter tax1 = makeCounter(CounterIds::TaxCounter1, 0);
    EXPECT_EQ(Server_Player::evaluateDelCounter(false, false, CounterIds::TaxCounter1, &tax1),
              Response::RespGameNotStarted);
}

TEST(EvaluateSetCounterActive, RejectsWhenGameNotStarted)
{
    Server_Counter counter = makeCounter(CounterIds::TaxCounter1, 0);
    EXPECT_EQ(Server_Player::evaluateSetCounterActive(/*gameStarted=*/false, /*playerConceded=*/false,
                                                      /*commandZoneEnabled=*/true, CounterIds::TaxCounter1, &counter,
                                                      /*requestedActive=*/true, nullptr, nullptr),
              Response::RespGameNotStarted);
}

TEST(EvaluateSetCounterActive, RejectsWhenPlayerConceded)
{
    Server_Counter counter = makeCounter(CounterIds::TaxCounter1, 0);
    EXPECT_EQ(Server_Player::evaluateSetCounterActive(true, /*playerConceded=*/true, true, CounterIds::TaxCounter1,
                                                      &counter, true, nullptr, nullptr),
              Response::RespContextError);
}

TEST(EvaluateSetCounterActive, RejectsNonTaxCounter)
{
    Server_Counter counter = makeCounter(UserCounterId, 0);
    EXPECT_EQ(
        Server_Player::evaluateSetCounterActive(true, false, true, UserCounterId, &counter, true, nullptr, nullptr),
        Response::RespFunctionNotAllowed);
}

TEST(EvaluateSetCounterActive, RejectsWhenCommandZoneDisabled)
{
    Server_Counter counter = makeCounter(CounterIds::TaxCounter1, 0);
    EXPECT_EQ(Server_Player::evaluateSetCounterActive(true, false, /*commandZoneEnabled=*/false,
                                                      CounterIds::TaxCounter1, &counter, true, nullptr, nullptr),
              Response::RespContextError);
}

TEST(EvaluateSetCounterActive, RejectsMissingCounter)
{
    EXPECT_EQ(Server_Player::evaluateSetCounterActive(true, false, true, CounterIds::TaxCounter1, nullptr, true,
                                                      nullptr, nullptr),
              Response::RespNameNotFound);
}

TEST(EvaluateSetCounterActive, RejectsDisablingWhenTaxAccumulated)
{
    Server_Counter counter = makeCounter(CounterIds::TaxCounter1, 3);
    EXPECT_EQ(Server_Player::evaluateSetCounterActive(true, false, true, CounterIds::TaxCounter1, &counter,
                                                      /*requestedActive=*/false, nullptr, nullptr),
              Response::RespContextError);
}

TEST(EvaluateSetCounterActive, AllowsEnablingWithAccumulatedTax)
{
    Server_Counter counter = makeCounter(CounterIds::TaxCounter1, 3);
    EXPECT_EQ(Server_Player::evaluateSetCounterActive(true, false, true, CounterIds::TaxCounter1, &counter,
                                                      /*requestedActive=*/true, nullptr, nullptr),
              Response::RespOk);
}

TEST(EvaluateSetCounterActive, AllowsDisablingWhenCounterIsZero)
{
    Server_Counter counter = makeCounter(CounterIds::TaxCounter1, 0);
    EXPECT_EQ(Server_Player::evaluateSetCounterActive(true, false, true, CounterIds::TaxCounter1, &counter,
                                                      /*requestedActive=*/false, nullptr, nullptr),
              Response::RespOk);
}

TEST(EvaluateSetCounterActive, AllowsEnabling2ndTaxWhen1stIsActive)
{
    Server_Counter tax1 = makeCounter(CounterIds::TaxCounter1, 0, true);
    Server_Counter tax2 = makeCounter(CounterIds::TaxCounter2, 0, false);
    EXPECT_EQ(Server_Player::evaluateSetCounterActive(true, false, true, CounterIds::TaxCounter2, &tax2,
                                                      /*requestedActive=*/true, &tax1, nullptr),
              Response::RespOk);
}

TEST(EvaluateSetCounterActive, RejectsEnabling2ndTaxWhen1stIsInactive)
{
    Server_Counter tax1 = makeCounter(CounterIds::TaxCounter1, 0, false);
    Server_Counter tax2 = makeCounter(CounterIds::TaxCounter2, 0, false);
    EXPECT_EQ(Server_Player::evaluateSetCounterActive(true, false, true, CounterIds::TaxCounter2, &tax2,
                                                      /*requestedActive=*/true, &tax1, nullptr),
              Response::RespContextError);
}

TEST(EvaluateSetCounterActive, AllowsDisabling1stTaxWhen2ndIsInactive)
{
    Server_Counter tax1 = makeCounter(CounterIds::TaxCounter1, 0, true);
    Server_Counter tax2 = makeCounter(CounterIds::TaxCounter2, 0, false);
    EXPECT_EQ(Server_Player::evaluateSetCounterActive(true, false, true, CounterIds::TaxCounter1, &tax1,
                                                      /*requestedActive=*/false, nullptr, &tax2),
              Response::RespOk);
}

TEST(EvaluateSetCounterActive, RejectsDisabling1stTaxWhen2ndIsActive)
{
    Server_Counter tax1 = makeCounter(CounterIds::TaxCounter1, 0, true);
    Server_Counter tax2 = makeCounter(CounterIds::TaxCounter2, 0, true);
    EXPECT_EQ(Server_Player::evaluateSetCounterActive(true, false, true, CounterIds::TaxCounter1, &tax1,
                                                      /*requestedActive=*/false, nullptr, &tax2),
              Response::RespContextError);
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

TEST(EvaluateCreateCounter, RejectsTaxCounterNames)
{
    EXPECT_EQ(Server_Player::evaluateCreateCounter(true, false, CounterNames::TaxCounter1),
              Response::RespFunctionNotAllowed);
    EXPECT_EQ(Server_Player::evaluateCreateCounter(true, false, CounterNames::TaxCounter2),
              Response::RespFunctionNotAllowed);
    EXPECT_EQ(Server_Player::evaluateCreateCounter(true, false, CounterNames::TaxCounter3),
              Response::RespFunctionNotAllowed);
    EXPECT_EQ(Server_Player::evaluateCreateCounter(true, false, CounterNames::TaxCounter4),
              Response::RespFunctionNotAllowed);
    EXPECT_EQ(Server_Player::evaluateCreateCounter(true, false, CounterNames::TaxCounter5),
              Response::RespFunctionNotAllowed);
}

TEST(EvaluateCreateCounter, AllowsUserCounterName)
{
    EXPECT_EQ(Server_Player::evaluateCreateCounter(true, false, "poison"), Response::RespOk);
}

TEST(EvaluateModifyCounter, RejectsWhenGameNotStarted)
{
    Server_Counter counter = makeCounter(UserCounterId, 0);
    EXPECT_EQ(Server_Player::evaluateModifyCounter(/*gameStarted=*/false, /*playerConceded=*/false, true, UserCounterId,
                                                   &counter),
              Response::RespGameNotStarted);
}

TEST(EvaluateModifyCounter, RejectsWhenPlayerConceded)
{
    Server_Counter counter = makeCounter(UserCounterId, 0);
    EXPECT_EQ(Server_Player::evaluateModifyCounter(true, /*playerConceded=*/true, true, UserCounterId, &counter),
              Response::RespContextError);
}

TEST(EvaluateModifyCounter, RejectsMissingCounter)
{
    EXPECT_EQ(Server_Player::evaluateModifyCounter(true, false, true, UserCounterId, nullptr),
              Response::RespNameNotFound);
}

TEST(EvaluateModifyCounter, AllowsUserCounter)
{
    Server_Counter counter = makeCounter(UserCounterId, 5);
    EXPECT_EQ(Server_Player::evaluateModifyCounter(true, false, true, UserCounterId, &counter), Response::RespOk);
}

TEST(EvaluateModifyCounter, RejectsTaxCounterWhenCommandZoneDisabled)
{
    Server_Counter counter = makeCounter(CounterIds::TaxCounter1, 0);
    EXPECT_EQ(Server_Player::evaluateModifyCounter(true, false, /*commandZoneEnabled=*/false, CounterIds::TaxCounter1,
                                                   &counter),
              Response::RespContextError);
}

TEST(EvaluateModifyCounter, RejectsInactiveTaxCounter)
{
    Server_Counter counter = makeCounter(CounterIds::TaxCounter2, 0, false);
    EXPECT_EQ(Server_Player::evaluateModifyCounter(true, false, /*commandZoneEnabled=*/true, CounterIds::TaxCounter2,
                                                   &counter),
              Response::RespContextError);
}

TEST(EvaluateModifyCounter, AllowsActiveTaxCounter)
{
    Server_Counter counter = makeCounter(CounterIds::TaxCounter1, 0, true);
    EXPECT_EQ(Server_Player::evaluateModifyCounter(true, false, /*commandZoneEnabled=*/true, CounterIds::TaxCounter1,
                                                   &counter),
              Response::RespOk);
}

TEST(EvaluateModifyCounter, RejectsMissingTaxCounter)
{
    EXPECT_EQ(Server_Player::evaluateModifyCounter(true, false, /*commandZoneEnabled=*/true, CounterIds::TaxCounter1,
                                                   nullptr),
              Response::RespNameNotFound);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
