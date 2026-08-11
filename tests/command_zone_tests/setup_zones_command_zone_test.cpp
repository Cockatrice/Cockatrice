/** @file setup_zones_command_zone_test.cpp
 *  @brief Tests for the command-zone branch of Server_Player::setupZones().
 *  @ingroup Tests
 */

#include "../movecard_tests/server_test_helpers.h"
#include "game/server_counter.h"
#include "game/server_game.h"
#include "game/server_player.h"
#include "server_room.h"

#include <gtest/gtest.h>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/protocol/pb/color.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_user.pb.h>
#include <libcockatrice/rng/rng_abstract.h>
#include <libcockatrice/utility/counter_ids.h>
#include <libcockatrice/utility/counter_limits.h>
#include <libcockatrice/utility/zone_names.h>

RNG_Abstract *rng = nullptr; // required by linked server code

namespace
{
/**
 * @brief A Server_Player whose protected deck member is populated with an empty deck.
 *
 * setupZones() reads deck->getCardNodes(); the base member is null by default, so this
 * subclass injects an empty deck. An empty deck makes the card-loading loops no-ops,
 * leaving only the zone/counter setup under test. The base destructor does not free deck
 * (only prepareDestroy() does, which the test never calls), so this subclass deletes it.
 */
struct CommandZoneTestPlayer : Server_Player
{
    CommandZoneTestPlayer(Server_Game *_game, const ServerInfo_User &_userInfo)
        : Server_Player(_game, 1, _userInfo, false, nullptr)
    {
        deck = new DeckList();
    }

    ~CommandZoneTestPlayer() override
    {
        delete deck;
        deck = nullptr;
    }
};

/**
 * @brief Builds a Server_Player on a minimal fake game whose command zone is toggled by the
 *        constructor argument, then runs setupZones() so getCounters() reflects the result.
 */
struct SetupFixture
{
    ServerInfo_User user;
    FakeServer server;
    Server_Room room{0, 0, "", "", "", "", false, "", {}, &server};
    Server_Game game;
    CommandZoneTestPlayer player;

    explicit SetupFixture(bool commandZoneEnabled)
        : game(user,
               1,
               "",
               "",
               2,
               QList<int>(),
               false,
               false,
               false,
               false,
               false,
               false,
               20,
               false,
               commandZoneEnabled,
               &room),
          player(&game, user)
    {
        player.setupZones();
    }

    ~SetupFixture()
    {
        player.clearZones(); // owns and deletes the counters created by setupZones()
    }
};
} // namespace

TEST(SetupZonesCommandZone, CreatesAllTaxCountersWhenEnabled)
{
    SetupFixture f(true);
    const QMap<int, Server_Counter *> &counters = f.player.getCounters();

    EXPECT_TRUE(f.player.getZones().contains(ZoneNames::COMMAND));

    for (int i = 0; i < CounterIds::TaxCounterCount; ++i) {
        int id = CounterIds::taxCounterIdFromIndex(i);
        ASSERT_TRUE(counters.contains(id));
    }

    const Server_Counter *tax1 = counters.value(CounterIds::TaxCounter1);
    EXPECT_TRUE(tax1->isActive());
    EXPECT_EQ(tax1->getCount(), 0);

    for (int i = 1; i < CounterIds::TaxCounterCount; ++i) {
        int id = CounterIds::taxCounterIdFromIndex(i);
        const Server_Counter *counter = counters.value(id);
        EXPECT_FALSE(counter->isActive());
        EXPECT_EQ(counter->getCount(), 0);
    }
}

TEST(SetupZonesCommandZone, TaxCountersUseBounds)
{
    SetupFixture f(true);
    Server_Counter *tax1 = f.player.getCounters().value(CounterIds::TaxCounter1);
    ASSERT_NE(tax1, nullptr);

    EXPECT_TRUE(tax1->setCount(MAX_COUNTER_VALUE + 1000));
    EXPECT_EQ(tax1->getCount(), MAX_COUNTER_VALUE);
    EXPECT_TRUE(tax1->setCount(-1));
    EXPECT_EQ(tax1->getCount(), 0);
}

TEST(SetupZonesCommandZone, NoTaxCountersWhenDisabled)
{
    SetupFixture f(false);
    const QMap<int, Server_Counter *> &counters = f.player.getCounters();

    EXPECT_FALSE(f.player.getZones().contains(ZoneNames::COMMAND));

    for (int i = 0; i < CounterIds::TaxCounterCount; ++i) {
        int id = CounterIds::taxCounterIdFromIndex(i);
        EXPECT_FALSE(counters.contains(id));
    }

    EXPECT_TRUE(counters.contains(0));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
