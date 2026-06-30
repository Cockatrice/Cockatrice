/** @file new_counter_id_test.cpp
 *  @brief Tests for Server_Player::newCounterId() id allocation.
 *  @ingroup Tests
 */

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

TEST(NewCounterId, ReturnsFirstUserIdWhenNoCounters)
{
    PlayerFixture f;
    EXPECT_EQ(f.player.newCounterId(), CounterIds::FirstUserId);
}

TEST(NewCounterId, SkipsReservedRangeWhenOnlyReservedCountersExist)
{
    PlayerFixture f;
    f.player.addCounter(new Server_Counter(3, "g", color(), 20, 0));
    f.player.addCounter(new Server_Counter(5, "h", color(), 20, 0));
    EXPECT_EQ(f.player.newCounterId(), CounterIds::FirstUserId);
}

TEST(NewCounterId, ReturnsNextIdAboveHighestUserCounter)
{
    PlayerFixture f;
    f.player.addCounter(new Server_Counter(CounterIds::FirstUserId, "a", color(), 20, 0)); // 10
    f.player.addCounter(new Server_Counter(15, "b", color(), 20, 0));
    EXPECT_EQ(f.player.newCounterId(), 16);
}

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
