#include "game/game_config.h"
#include "game/server_abstract_participant.h"
#include "game/server_game.h"
#include "server_abstractuserinterface.h"
#include "server_response_containers.h"
#include "server_room.h"
#include "server_test_helpers.h"

#include <gtest/gtest.h>
#include <libcockatrice/protocol/pb/game_event_container.pb.h>
#include <libcockatrice/protocol/pb/response.pb.h>
#include <libcockatrice/protocol/pb/room_event.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_user.pb.h>
#include <libcockatrice/protocol/pb/session_event.pb.h>
#include <libcockatrice/rng/rng_abstract.h>

RNG_Abstract *rng = nullptr; // this needs to be defined due to other functions in server

class ScriptedRng : public RNG_Abstract
{
public:
    QList<unsigned int> values;

    unsigned int rand(int min, int max) override
    {
        const unsigned int value = values.takeFirst();
        EXPECT_GE(value, static_cast<unsigned int>(min));
        EXPECT_LE(value, static_cast<unsigned int>(max));
        return value;
    }
};

class FakeUserInterface : public Server_AbstractUserInterface
{
public:
    FakeUserInterface(Server *_server, const ServerInfo_User &_userInfo) : Server_AbstractUserInterface(_server)
    {
        setUserInfo(_userInfo);
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

    void sendProtocolItem(const GameEventContainer &) override
    {
    }

    void sendProtocolItem(const RoomEvent &) override
    {
    }
};

namespace
{
QStringList participantNamesInOrder(const Server_Game &game)
{
    QStringList names;
    for (const auto *participant : game.getParticipants()) {
        names.append(QString::fromStdString(participant->getUserInfo()->name()));
    }
    return names;
}

QList<int> participantIdsInOrder(const Server_Game &game)
{
    QList<int> ids;
    for (int id : game.getParticipants().keys()) {
        ids.append(id);
    }
    return ids;
}
} // namespace

class PlayerOrderTest : public ::testing::Test
{
protected:
    ServerInfo_User creator;
    ServerInfo_User userB;
    ServerInfo_User userC;

    PlayerOrderTest()
    {
        creator.set_name("A");
        creator.set_user_level(ServerInfo_User::IsRegistered);
        userB.set_name("B");
        userB.set_user_level(ServerInfo_User::IsRegistered);
        userC.set_name("C");
        userC.set_user_level(ServerInfo_User::IsRegistered);
    }
};

TEST_F(PlayerOrderTest, JoinOrderIsSeatOrder)
{
    FakeServer server;
    Server_Room room(0, 0, "", "", "", "", false, "", {}, &server);
    FakeUserInterface uiA(&server, creator);
    FakeUserInterface uiB(&server, userB);
    FakeUserInterface uiC(&server, userC);
    GameConfig config{.creatorInfo = creator,
                      .gameId = 1,
                      .description = QString(),
                      .password = QString(),
                      .maxPlayers = 3,
                      .gameTypes = QList<int>(),
                      .onlyBuddies = false,
                      .onlyRegistered = false,
                      .spectatorsAllowed = false,
                      .spectatorsNeedPassword = false,
                      .spectatorsCanTalk = false,
                      .spectatorsSeeEverything = false,
                      .startingLifeTotal = 20,
                      .shareDecklistsOnLoad = false,
                      .shufflePlayers = false};
    Server_Game game(config, &room);
    ResponseContainer rc(0);

    game.addPlayer(&uiA, rc, false, false);
    game.addPlayer(&uiB, rc, false, false);
    game.addPlayer(&uiC, rc, false, false);

    EXPECT_EQ(participantNamesInOrder(game), QStringList({"A", "B", "C"}));
    EXPECT_EQ(participantIdsInOrder(game), QList<int>({0, 1, 2}));
    EXPECT_EQ(game.getHostId(), 0);
    EXPECT_EQ(game.getPlayerCount(), 3);
}

TEST_F(PlayerOrderTest, ReorderPlayerSeats)
{
    FakeServer server;
    Server_Room room(0, 0, "", "", "", "", false, "", {}, &server);
    FakeUserInterface uiA(&server, creator);
    FakeUserInterface uiB(&server, userB);
    FakeUserInterface uiC(&server, userC);
    GameConfig config{.creatorInfo = creator,
                      .gameId = 1,
                      .description = QString(),
                      .password = QString(),
                      .maxPlayers = 3,
                      .gameTypes = QList<int>(),
                      .onlyBuddies = false,
                      .onlyRegistered = false,
                      .spectatorsAllowed = false,
                      .spectatorsNeedPassword = false,
                      .spectatorsCanTalk = false,
                      .spectatorsSeeEverything = false,
                      .startingLifeTotal = 20,
                      .shareDecklistsOnLoad = false,
                      .shufflePlayers = false};
    Server_Game game(config, &room);
    ResponseContainer rc(0);

    game.addPlayer(&uiA, rc, false, false);
    game.addPlayer(&uiB, rc, false, false);
    game.addPlayer(&uiC, rc, false, false);

    game.reorderPlayerSeats({"C", "A", "B"});

    const QList<int> ids = participantIdsInOrder(game);
    EXPECT_EQ(participantNamesInOrder(game), QStringList({"C", "A", "B"}));
    EXPECT_EQ(ids.size(), 3);
    EXPECT_GE(ids.at(0), 3); // re-seated players get fresh ids
    EXPECT_LT(ids.at(0), ids.at(1));
    EXPECT_LT(ids.at(1), ids.at(2));

    // the host seat follows the creator
    EXPECT_EQ(game.getHostId(), ids.at(1));

    // every user interface's game list is updated to the new player id
    const int gameId = game.getGameId();
    EXPECT_EQ(uiA.getGames().value(gameId).second, ids.at(1));
    EXPECT_EQ(uiB.getGames().value(gameId).second, ids.at(2));
    EXPECT_EQ(uiC.getGames().value(gameId).second, ids.at(0));
}

TEST_F(PlayerOrderTest, ReorderPlayerSeatsKeepsPlayerCount)
{
    FakeServer server;
    Server_Room room(0, 0, "", "", "", "", false, "", {}, &server);
    FakeUserInterface uiA(&server, creator);
    FakeUserInterface uiB(&server, userB);
    FakeUserInterface uiC(&server, userC);
    GameConfig config{.creatorInfo = creator,
                      .gameId = 1,
                      .description = QString(),
                      .password = QString(),
                      .maxPlayers = 3,
                      .gameTypes = QList<int>(),
                      .onlyBuddies = false,
                      .onlyRegistered = false,
                      .spectatorsAllowed = false,
                      .spectatorsNeedPassword = false,
                      .spectatorsCanTalk = false,
                      .spectatorsSeeEverything = false,
                      .startingLifeTotal = 20,
                      .shareDecklistsOnLoad = false,
                      .shufflePlayers = false};
    Server_Game game(config, &room);
    ResponseContainer rc(0);

    game.addPlayer(&uiA, rc, false, false);
    game.addPlayer(&uiB, rc, false, false);
    game.addPlayer(&uiC, rc, false, false);

    // listing only a subset appends the remaining players in their current order
    game.reorderPlayerSeats({"C"});

    EXPECT_EQ(participantNamesInOrder(game), QStringList({"C", "A", "B"}));
    EXPECT_EQ(game.getPlayerCount(), 3);
    EXPECT_EQ(game.getHostId(), participantIdsInOrder(game).at(1));
}

TEST_F(PlayerOrderTest, ShufflePlayerSeatsIsAPermutation)
{
    FakeServer server;
    Server_Room room(0, 0, "", "", "", "", false, "", {}, &server);
    FakeUserInterface uiA(&server, creator);
    FakeUserInterface uiB(&server, userB);
    FakeUserInterface uiC(&server, userC);
    GameConfig config{.creatorInfo = creator,
                      .gameId = 1,
                      .description = QString(),
                      .password = QString(),
                      .maxPlayers = 3,
                      .gameTypes = QList<int>(),
                      .onlyBuddies = false,
                      .onlyRegistered = false,
                      .spectatorsAllowed = false,
                      .spectatorsNeedPassword = false,
                      .spectatorsCanTalk = false,
                      .spectatorsSeeEverything = false,
                      .startingLifeTotal = 20,
                      .shareDecklistsOnLoad = false,
                      .shufflePlayers = false};
    Server_Game game(config, &room);
    ResponseContainer rc(0);

    game.addPlayer(&uiA, rc, false, false);
    game.addPlayer(&uiB, rc, false, false);
    game.addPlayer(&uiC, rc, false, false);

    // fisher-yates on [A, B, C]: rand(0, 2) -> 1 (take B), rand(0, 1) -> 0 (take A), rand(0, 0) -> 0 (take C)
    auto *scriptedRng = new ScriptedRng;
    scriptedRng->values = {1, 0, 0};
    rng = scriptedRng;

    game.shufflePlayerSeats();

    EXPECT_EQ(participantNamesInOrder(game), QStringList({"B", "A", "C"}));
    const QList<int> ids = participantIdsInOrder(game);
    EXPECT_EQ(ids.size(), 3);
    QSet<int> idSet(ids.cbegin(), ids.cend());
    EXPECT_EQ(idSet.size(), 3); // ids are unique
    EXPECT_EQ(game.getHostId(), ids.at(1));
}
