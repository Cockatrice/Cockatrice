/** @file server_transform_context_test.cpp
 *  @brief Tests that the card transform path attaches a Context_TransformCard
 *         to the event storage, so the client can render a single
 *         "Player X transforms card Y into card Z" chat message.
 *  @ingroup Tests
 */

#include "game/game_config.h"
#include "game/server_abstract_player.h"
#include "game/server_card.h"
#include "game/server_cardzone.h"
#include "game/server_game.h"
#include "movecard_tests/server_test_helpers.h"
#include "server_response_containers.h"
#include "server_room.h"

#include <QCoreApplication>
#include <gtest/gtest.h>
#include <libcockatrice/protocol/pb/command_create_token.pb.h>
#include <libcockatrice/protocol/pb/context_transform_card.pb.h>
#include <libcockatrice/protocol/pb/event_create_token.pb.h>
#include <libcockatrice/protocol/pb/event_destroy_card.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_user.pb.h>
#include <libcockatrice/rng/rng_abstract.h>
#include <libcockatrice/utility/zone_names.h>

RNG_Abstract *rng = nullptr; // referenced by the server_remote library

namespace
{

class TransformContextTest : public ::testing::Test
{
protected:
    ServerInfo_User user;
    FakeServer server;
    std::unique_ptr<Server_Room> room;
    std::unique_ptr<Server_Game> game;
    std::unique_ptr<Server_AbstractPlayer> player;
    Server_Card *targetCard = nullptr;

    void SetUp() override
    {
        user.set_name("test-user");

        room = std::make_unique<Server_Room>(0, 0, "", "", "", "", false, "", QList<QString>(), &server);
        GameConfig config{.creatorInfo = user,
                          .gameId = 1,
                          .description = QString(),
                          .password = QString(),
                          .maxPlayers = 2,
                          .gameTypes = QList<int>(),
                          .onlyBuddies = false,
                          .onlyRegistered = false,
                          .spectatorsAllowed = false,
                          .spectatorsNeedPassword = false,
                          .spectatorsCanTalk = false,
                          .spectatorsSeeEverything = false,
                          .startingLifeTotal = 20,
                          .shareDecklistsOnLoad = false};
        game = std::make_unique<Server_Game>(config, room.get());

        // Start the game with no participants so cmdCreateToken can run; the
        // start is emitted via a queued connection, so process posted events.
        game->startGameIfReady(true);
        QCoreApplication::processEvents();
        ASSERT_TRUE(game->getGameStarted());

        player = std::make_unique<Server_AbstractPlayer>(game.get(), 1, user, false, nullptr);
        auto *tableZone = new Server_CardZone(player.get(), ZoneNames::TABLE, true, ServerInfo_Zone::PublicZone);
        player->addZone(tableZone);

        targetCard = new Server_Card({"Old Card", "old-card"}, player->newCardId(), 0, 0);
        tableZone->insertCard(targetCard, 0, 0);
    }
};

Server_CardZone *tableZoneOf(Server_AbstractPlayer &player)
{
    return player.getZones().value(ZoneNames::TABLE);
}

Command_CreateToken makeTransformCommand(uint32_t targetCardId)
{
    Command_CreateToken cmd;
    cmd.set_zone(ZoneNames::TABLE);
    cmd.set_target_zone(ZoneNames::TABLE);
    cmd.set_target_card_id(targetCardId);
    cmd.set_target_mode(Command_CreateToken::TRANSFORM_INTO);
    cmd.set_card_name("New Card");
    cmd.set_card_provider_id("new-card");
    cmd.set_color("m");
    cmd.set_pt("2/2");
    cmd.set_destroy_on_zone_change(true);
    cmd.set_x(-1);
    cmd.set_y(-1);
    return cmd;
}

TEST_F(TransformContextTest, TransformCommandSetsTransformContext)
{
    GameEventStorage ges;
    ResponseContainer rc(0);

    const auto response = player->cmdCreateToken(makeTransformCommand(targetCard->getId()), rc, ges);

    EXPECT_EQ(response, Response::RespOk);
    ASSERT_NE(ges.getGameEventContext(), nullptr);
    EXPECT_TRUE(static_cast<GameEventContext *>(ges.getGameEventContext())->HasExtension(Context_TransformCard::ext));
}

TEST_F(TransformContextTest, TransformEventSequenceDestroyThenCreate)
{
    GameEventStorage ges;
    ResponseContainer rc(0);

    ASSERT_EQ(player->cmdCreateToken(makeTransformCommand(targetCard->getId()), rc, ges), Response::RespOk);

    QList<uint32_t> destroyedIds;
    QList<Event_CreateToken> createdTokens;
    for (const auto *item : ges.getGameEventList()) {
        const GameEvent &event = item->getGameEvent();
        if (event.HasExtension(Event_DestroyCard::ext)) {
            destroyedIds.append(event.GetExtension(Event_DestroyCard::ext).card_id());
        }
        if (event.HasExtension(Event_CreateToken::ext)) {
            createdTokens.append(event.GetExtension(Event_CreateToken::ext));
        }
    }

    // The old card is destroyed and exactly one token is created in its place.
    EXPECT_EQ(destroyedIds, QList<uint32_t>({static_cast<uint32_t>(targetCard->getId())}));
    ASSERT_EQ(createdTokens.size(), 1);
    EXPECT_EQ(createdTokens.first().card_name(), "New Card");
    EXPECT_EQ(createdTokens.first().pt(), "2/2");
    EXPECT_EQ(createdTokens.first().destroy_on_zone_change(), true);

    // The old card is removed from the table and the new card replaces it.
    EXPECT_EQ(tableZoneOf(*player)->getCard(targetCard->getId()), nullptr);
    EXPECT_NE(tableZoneOf(*player)->getCard(createdTokens.first().card_id()), nullptr);
}

TEST_F(TransformContextTest, PlainTokenCreateHasNoTransformContext)
{
    Command_CreateToken cmd;
    cmd.set_zone(ZoneNames::TABLE);
    cmd.set_card_name("Plain Token");
    cmd.set_card_provider_id("plain-token");
    cmd.set_color("");
    cmd.set_pt("1/1");
    cmd.set_destroy_on_zone_change(true);
    cmd.set_x(-1);
    cmd.set_y(-1);

    GameEventStorage ges;
    ResponseContainer rc(0);

    EXPECT_EQ(player->cmdCreateToken(cmd, rc, ges), Response::RespOk);
    EXPECT_EQ(ges.getGameEventContext(), nullptr);
}

} // namespace

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}