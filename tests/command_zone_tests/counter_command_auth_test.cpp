// Unit tests for the pure authorization logic extracted from the counter command
// handlers. Server_Player::evaluateDelCounter() and evaluateSetCounterActive() are
// static and depend only on their arguments, so each guard branch can be exercised
// directly without a started game, network stack, or player fixture.

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
// A user-range counter id that is never a reserved tax counter.
constexpr int UserCounterId = CounterIds::FirstUserId;

// Builds a counter with the given id and current value; the other fields are
// irrelevant to the authorization decisions under test.
Server_Counter makeCounter(int id, int count)
{
    return Server_Counter(id, "c", color(), 20, count);
}
} // namespace

// ---------------------------------------------------------------------------
// evaluateDelCounter
// ---------------------------------------------------------------------------

// The game must be started before any counter can be deleted.
TEST(EvaluateDelCounter, RejectsWhenGameNotStarted)
{
    Server_Counter counter = makeCounter(UserCounterId, 0);
    EXPECT_EQ(
        Server_Player::evaluateDelCounter(/*gameStarted=*/false, /*playerConceded=*/false, UserCounterId, &counter),
        Response::RespGameNotStarted);
}

// A conceded player may no longer mutate counters.
TEST(EvaluateDelCounter, RejectsWhenPlayerConceded)
{
    Server_Counter counter = makeCounter(UserCounterId, 0);
    EXPECT_EQ(Server_Player::evaluateDelCounter(/*gameStarted=*/true, /*playerConceded=*/true, UserCounterId, &counter),
              Response::RespContextError);
}

// Reserved tax counters are server-managed and may never be deleted by a client.
TEST(EvaluateDelCounter, RejectsTaxCounters)
{
    Server_Counter commander = makeCounter(CounterIds::CommanderTax, 0);
    EXPECT_EQ(Server_Player::evaluateDelCounter(true, false, CounterIds::CommanderTax, &commander),
              Response::RespFunctionNotAllowed);

    Server_Counter partner = makeCounter(CounterIds::PartnerTax, 0);
    EXPECT_EQ(Server_Player::evaluateDelCounter(true, false, CounterIds::PartnerTax, &partner),
              Response::RespFunctionNotAllowed);
}

// A non-existent counter (null lookup) is reported as not found.
TEST(EvaluateDelCounter, RejectsMissingCounter)
{
    EXPECT_EQ(Server_Player::evaluateDelCounter(true, false, UserCounterId, nullptr), Response::RespNameNotFound);
}

// A live user counter in a started, non-conceded game may be deleted.
TEST(EvaluateDelCounter, AllowsDeletingUserCounter)
{
    Server_Counter counter = makeCounter(UserCounterId, 7);
    EXPECT_EQ(Server_Player::evaluateDelCounter(true, false, UserCounterId, &counter), Response::RespOk);
}

// The game-not-started guard is checked before the tax-counter guard: an attempt
// to delete a tax counter before the game starts reports RespGameNotStarted.
TEST(EvaluateDelCounter, GameNotStartedTakesPrecedenceOverTaxGuard)
{
    Server_Counter commander = makeCounter(CounterIds::CommanderTax, 0);
    EXPECT_EQ(Server_Player::evaluateDelCounter(false, false, CounterIds::CommanderTax, &commander),
              Response::RespGameNotStarted);
}

// ---------------------------------------------------------------------------
// evaluateSetCounterActive
// ---------------------------------------------------------------------------

// The game must be started before a counter's active state can change.
TEST(EvaluateSetCounterActive, RejectsWhenGameNotStarted)
{
    Server_Counter counter = makeCounter(CounterIds::CommanderTax, 0);
    EXPECT_EQ(Server_Player::evaluateSetCounterActive(/*gameStarted=*/false, /*playerConceded=*/false,
                                                      /*commandZoneEnabled=*/true, CounterIds::CommanderTax, &counter,
                                                      /*requestedActive=*/true),
              Response::RespGameNotStarted);
}

// A conceded player may no longer toggle counters.
TEST(EvaluateSetCounterActive, RejectsWhenPlayerConceded)
{
    Server_Counter counter = makeCounter(CounterIds::CommanderTax, 0);
    EXPECT_EQ(Server_Player::evaluateSetCounterActive(true, /*playerConceded=*/true, true, CounterIds::CommanderTax,
                                                      &counter, true),
              Response::RespContextError);
}

// Only the reserved tax counters support an active/inactive toggle; user counters
// are rejected as a disallowed operation.
TEST(EvaluateSetCounterActive, RejectsNonTaxCounter)
{
    Server_Counter counter = makeCounter(UserCounterId, 0);
    EXPECT_EQ(Server_Player::evaluateSetCounterActive(true, false, true, UserCounterId, &counter, true),
              Response::RespFunctionNotAllowed);
}

// Toggling a tax counter is only meaningful when the command zone is enabled.
TEST(EvaluateSetCounterActive, RejectsWhenCommandZoneDisabled)
{
    Server_Counter counter = makeCounter(CounterIds::CommanderTax, 0);
    EXPECT_EQ(Server_Player::evaluateSetCounterActive(true, false, /*commandZoneEnabled=*/false,
                                                      CounterIds::CommanderTax, &counter, true),
              Response::RespContextError);
}

// A missing counter is reported as not found.
TEST(EvaluateSetCounterActive, RejectsMissingCounter)
{
    EXPECT_EQ(Server_Player::evaluateSetCounterActive(true, false, true, CounterIds::CommanderTax, nullptr, true),
              Response::RespNameNotFound);
}

// A tax counter may not be disabled while it still holds accumulated tax; the
// player must reset it to zero first.
TEST(EvaluateSetCounterActive, RejectsDisablingWhenTaxAccumulated)
{
    Server_Counter counter = makeCounter(CounterIds::CommanderTax, 3);
    EXPECT_EQ(Server_Player::evaluateSetCounterActive(true, false, true, CounterIds::CommanderTax, &counter,
                                                      /*requestedActive=*/false),
              Response::RespContextError);
}

// Enabling a tax counter is allowed regardless of its current value.
TEST(EvaluateSetCounterActive, AllowsEnablingWithAccumulatedTax)
{
    Server_Counter counter = makeCounter(CounterIds::CommanderTax, 3);
    EXPECT_EQ(Server_Player::evaluateSetCounterActive(true, false, true, CounterIds::CommanderTax, &counter,
                                                      /*requestedActive=*/true),
              Response::RespOk);
}

// Disabling a tax counter that is already at zero is permitted.
TEST(EvaluateSetCounterActive, AllowsDisablingWhenCounterIsZero)
{
    Server_Counter counter = makeCounter(CounterIds::CommanderTax, 0);
    EXPECT_EQ(Server_Player::evaluateSetCounterActive(true, false, true, CounterIds::CommanderTax, &counter,
                                                      /*requestedActive=*/false),
              Response::RespOk);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
