#include "../movecard_tests/server_test_helpers.h"
#include "game/server_counter.h"
#include "game/server_game.h"
#include "game/server_player.h"
#include "server_room.h"

#include <gtest/gtest.h>
#include <libcockatrice/protocol/pb/color.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_user.pb.h>
#include <libcockatrice/rng/rng_abstract.h>
#include <libcockatrice/utility/counter_ids.h>

RNG_Abstract *rng = nullptr; // required by linked server code

namespace
{
// Wires up a Server_Player backed by a minimal fake game so that counters can be
// added directly via addCounter() to exercise newCounterId() in isolation.
// newCounterId() depends only on the player's counter map, not on game state,
// so the game never needs to be started.
struct PlayerFixture
{
    ServerInfo_User user;
    FakeServer server;
    Server_Room room{0, 0, "", "", "", "", false, "", {}, &server};
    Server_Game game{user,  1,     "",    "",    2,  QList<int>(), false, false,
                     false, false, false, false, 20, false,        false, &room};
    Server_Player player{&game, 1, user, false, nullptr};

    ~PlayerFixture()
    {
        player.clearZones(); // owns and deletes any counters added during the test
    }
};
} // namespace

// With no counters at all, the first allocated id must be FirstUserId, never 0.
TEST(NewCounterId, ReturnsFirstUserIdWhenNoCounters)
{
    PlayerFixture f;
    EXPECT_EQ(f.player.newCounterId(), CounterIds::FirstUserId);
}

// Reserved ids 0-9 (standard player counters and the commander tax counters) must
// not drag a new user counter down into the reserved range.
TEST(NewCounterId, SkipsReservedRangeWhenOnlyReservedCountersExist)
{
    PlayerFixture f;
    f.player.addCounter(new Server_Counter(3, "g", color(), 20, 0));
    f.player.addCounter(new Server_Counter(CounterIds::PartnerTax, CounterNames::PartnerTax, color(), 20, 0));
    EXPECT_EQ(f.player.newCounterId(), CounterIds::FirstUserId);
}

// Once user counters exist, the next id is one above the highest of them.
TEST(NewCounterId, ReturnsNextIdAboveHighestUserCounter)
{
    PlayerFixture f;
    f.player.addCounter(new Server_Counter(CounterIds::FirstUserId, "a", color(), 20, 0)); // 10
    f.player.addCounter(new Server_Counter(15, "b", color(), 20, 0));
    EXPECT_EQ(f.player.newCounterId(), 16);
}

// A reserved counter present alongside user counters must not affect the result.
TEST(NewCounterId, IgnoresReservedCountersWhenUserCountersPresent)
{
    PlayerFixture f;
    f.player.addCounter(new Server_Counter(5, "r", color(), 20, 0));     // reserved range
    f.player.addCounter(new Server_Counter(11, "user", color(), 20, 0)); // user range
    EXPECT_EQ(f.player.newCounterId(), 12);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
