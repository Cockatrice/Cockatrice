#include "game/game_config.h"
#include "game/server_abstract_player.h"
#include "game/server_card.h"
#include "game/server_cardzone.h"
#include "game/server_game.h"
#include "movecard_tests/server_test_helpers.h"
#include "server_abstractuserinterface.h"
#include "server_response_containers.h"
#include "server_room.h"

#include <QCoreApplication>
#include <gtest/gtest.h>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/protocol/pb/command_deck_select.pb.h>
#include <libcockatrice/protocol/pb/command_reveal_cards.pb.h>
#include <libcockatrice/protocol/pb/event_reveal_cards.pb.h>
#include <libcockatrice/protocol/pb/game_event_container.pb.h>
#include <libcockatrice/rng/rng_abstract.h>
#include <libcockatrice/utility/zone_names.h>
#include <memory>

namespace
{
class FirstRng : public RNG_Abstract
{
    unsigned int rand(int min, int) override
    {
        return min;
    }
} firstRng;

class User : public Server_AbstractUserInterface
{
public:
    User(Server *server, const char *name) : Server_AbstractUserInterface(server)
    {
        ServerInfo_User info;
        info.set_name(name);
        setUserInfo(info);
    }
    int getLastCommandTime() const override
    {
        return 0;
    }
    bool addSaidMessageSize(int) override
    {
        return true;
    }
    void sendProtocolItem(const Response &) override
    {
    }
    void sendProtocolItem(const SessionEvent &) override
    {
    }
    void sendProtocolItem(const RoomEvent &) override
    {
    }
    void sendProtocolItem(const GameEventContainer &) override
    {
    }
};

class RandomReveal : public ::testing::Test
{
protected:
    FakeServer server;
    User owner{&server, "Owner"}, target{&server, "Target"};
    Server_Room room{0, 0, "", "", "", "", false, "", {}, &server};
    std::unique_ptr<Server_Game> game;
    Server_AbstractPlayer *player;
    Server_CardZone *hand;
    ResponseContainer response{1};

    void SetUp() override
    {
        GameConfig config;
        config.creatorInfo = *owner.getUserInfo();
        config.gameId = 1;
        config.maxPlayers = 2;
        config.shareDecklistsOnLoad = false;
        game = std::make_unique<Server_Game>(config, &room);
        game->addPlayer(&owner, response, {});
        game->addPlayer(&target, response, {});
        for (auto *participant : game->getPlayers()) {
            GameCommand command;
            command.MutableExtension(Command_DeckSelect::ext)
                ->set_deck(DeckList().writeToString_Native().toStdString());
            GameEventStorage events;
            ASSERT_EQ(participant->processGameCommand(command, response, events), Response::RespOk);
            participant->setReadyStart(true);
        }
        game->startGameIfReady(false);
        QCoreApplication::processEvents();
        ASSERT_TRUE(game->getGameStarted());
        player = game->getPlayer(0);
        hand = player->getZones().value(ZoneNames::HAND);
        for (int i = 0; i < 4; ++i) {
            hand->insertCard(
                new Server_Card({.name = "Twin", .providerId = QString::number(i)}, player->newCardId(), i, 0), -1, 0);
        }
    }

    Response::ResponseCode reveal(int count, GameEventStorage &events)
    {
        GameCommand command;
        auto *request = command.MutableExtension(Command_RevealRandomCards::ext);
        request->set_zone_name(ZoneNames::HAND);
        request->set_player_id(1);
        request->set_number_of_cards(count);
        return player->processGameCommand(command, response, events);
    }
};

TEST_F(RandomReveal, DistinctPhysicalCardsArePrivateAndLeaveTheHandUnchanged)
{
    const auto original = hand->getCards();
    GameEventStorage events;
    ASSERT_EQ(reveal(2, events), Response::RespOk);
    ASSERT_EQ(events.getGameEventList().size(), 2);
    EXPECT_EQ(events.getPrivatePlayerId(), 1);
    const auto *privateEvent = events.getGameEventList().at(0);
    EXPECT_EQ(privateEvent->getRecipients(), GameEventStorageItem::SendToPrivate);
    const auto &cards = privateEvent->getGameEvent().GetExtension(Event_RevealCards::ext);
    ASSERT_EQ(cards.cards_size(), 2);
    EXPECT_NE(cards.cards(0).id(), cards.cards(1).id());
    const auto *publicEvent = events.getGameEventList().at(1);
    EXPECT_EQ(publicEvent->getRecipients(), GameEventStorageItem::SendToOthers);
    const auto &notice = publicEvent->getGameEvent().GetExtension(Event_RevealCards::ext);
    EXPECT_EQ(notice.cards_size(), 0);
    EXPECT_EQ(notice.number_of_cards(), 2);
    EXPECT_EQ(hand->getCards(), original);
}

TEST_F(RandomReveal, InvalidCountsDoNotQueueAnyReveal)
{
    for (int count : {0, -1, 5}) {
        GameEventStorage events;
        EXPECT_NE(reveal(count, events), Response::RespOk);
        EXPECT_TRUE(events.getGameEventList().isEmpty());
    }
}
} // namespace

RNG_Abstract *rng = &firstRng;

int main(int argc, char **argv)
{
    QCoreApplication application(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
