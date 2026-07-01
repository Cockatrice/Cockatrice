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

TEST(SetupZonesCommandZone, CreatesTaxCountersWhenEnabled)
{
    SetupFixture f(true);
    const QMap<int, Server_Counter *> &counters = f.player.getCounters();

    EXPECT_TRUE(f.player.getZones().contains(ZoneNames::COMMAND));
    ASSERT_TRUE(counters.contains(CounterIds::CommanderTax));
    ASSERT_TRUE(counters.contains(CounterIds::PartnerTax));

    const Server_Counter *commander = counters.value(CounterIds::CommanderTax);
    const Server_Counter *partner = counters.value(CounterIds::PartnerTax);

    EXPECT_TRUE(commander->isActive());
    EXPECT_FALSE(partner->isActive());
    EXPECT_EQ(commander->getCount(), 0);
    EXPECT_EQ(partner->getCount(), 0);
}

TEST(SetupZonesCommandZone, TaxCountersUseCommanderBounds)
{
    SetupFixture f(true);
    Server_Counter *commander = f.player.getCounters().value(CounterIds::CommanderTax);
    ASSERT_NE(commander, nullptr);

    EXPECT_TRUE(commander->setCount(MAX_COUNTER_VALUE + 1000));
    EXPECT_EQ(commander->getCount(), MAX_COUNTER_VALUE);
    EXPECT_TRUE(commander->setCount(-1));
    EXPECT_EQ(commander->getCount(), 0);
}

TEST(SetupZonesCommandZone, NoTaxCountersWhenDisabled)
{
    SetupFixture f(false);
    const QMap<int, Server_Counter *> &counters = f.player.getCounters();

    EXPECT_FALSE(f.player.getZones().contains(ZoneNames::COMMAND));
    EXPECT_FALSE(counters.contains(CounterIds::CommanderTax));
    EXPECT_FALSE(counters.contains(CounterIds::PartnerTax));
    EXPECT_TRUE(counters.contains(0));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
