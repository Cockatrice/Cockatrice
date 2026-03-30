#include "game/server_abstract_player.h"
#include "game/server_card.h"
#include "game/server_cardzone.h"
#include "game/server_game.h"
#include "server_response_containers.h"
#include "server_room.h"
#include "server_test_helpers.h"

#include <gtest/gtest.h>
#include <libcockatrice/protocol/pb/command_move_card.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_user.pb.h>
#include <libcockatrice/rng/rng_abstract.h>
#include <libcockatrice/utility/zone_names.h>

RNG_Abstract *rng = nullptr; // this needs to be defined due to other functions in server

TEST(ReverseCardMoveTest, MoveCardFromBottomTest)
{
    ServerInfo_User user;
    user.set_name("test-user");

	// instatiate necessary dependencies of the library
    FakeServer server;
    Server_Room room(0, 0, "", "", "", "", false, "", {}, &server);
    Server_Game game(user, 1, "", "", 2, QList<int>(), false, false, false, false, false, false, 20, false, &room);
    Server_AbstractPlayer player(&game, 1, user, false, nullptr);
    Server_CardZone deckZone(&player, ZoneNames::DECK, true, ServerInfo_Zone::PublicZone);
    Server_CardZone exileZone(&player, ZoneNames::EXILE, true, ServerInfo_Zone::PublicZone);

    // setup the deck with 20 useless cards
    for (int i = 0; i < 20; i++) {
        auto *cardUseless = new Server_Card({"Card Useless", "card-Useless"}, player.newCardId(), i, 0);
        deckZone.insertCard(cardUseless, i, 0);
    }

    // add 4 cards to the end of it
    auto *cardA = new Server_Card({"Card A", "card-a"}, player.newCardId(), 20, 0);
    auto *cardB = new Server_Card({"Card B", "card-b"}, player.newCardId(), 21, 0);
    auto *cardC = new Server_Card({"Card C", "card-c"}, player.newCardId(), 22, 0);
    auto *cardD = new Server_Card({"Card D", "card-d"}, player.newCardId(), 23, 0);

    deckZone.insertCard(cardA, 20, 0);
    deckZone.insertCard(cardB, 21, 0);
    deckZone.insertCard(cardC, 22, 0);
    deckZone.insertCard(cardD, 23, 0);

    // try to move them, with the expected client given order (n-3, n-2, n-1, n)
    CardToMove moveA;
    moveA.set_card_id(cardA->getId());
    CardToMove moveB;
    moveB.set_card_id(cardB->getId());
    CardToMove moveC;
    moveC.set_card_id(cardC->getId());
    CardToMove moveD;
    moveD.set_card_id(cardD->getId());

    QList<const CardToMove *> cardsToMove = {&moveA, &moveB, &moveC, &moveD};
    GameEventStorage ges;

    const auto response = player.moveCard(ges, &deckZone, cardsToMove, &exileZone, 0, 0, false, false, false, true);

    EXPECT_EQ(response, Response::RespOk);

    int positionA;
    int positionB;
    int positionC;
    int positionD;
    // check if they are on the destination zone
    EXPECT_EQ(exileZone.getCard(cardA->getId(), &positionA), cardA);
    EXPECT_EQ(exileZone.getCard(cardB->getId(), &positionB), cardB);
    EXPECT_EQ(exileZone.getCard(cardC->getId(), &positionC), cardC);
    EXPECT_EQ(exileZone.getCard(cardD->getId(), &positionD), cardD);

    // check if they are with the expected coordinates
    EXPECT_EQ(cardA->getX(), 3);
    EXPECT_EQ(cardB->getX(), 2);
    EXPECT_EQ(cardC->getX(), 1);
    EXPECT_EQ(cardD->getX(), 0);

    // check if they are with the expected positions
    EXPECT_EQ(positionA, 3);
    EXPECT_EQ(positionB, 2);
    EXPECT_EQ(positionC, 1);
    EXPECT_EQ(positionD, 0);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
