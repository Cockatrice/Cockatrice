#include "game_test_fixture.h"

#include "gtest/gtest.h"
#include <QString>
#include <libcockatrice/game/player/player_actions.h>
#include <libcockatrice/game/zones/table_zone_logic.h>
#include <libcockatrice/protocol/pb/command_create_token.pb.h>
#include <libcockatrice/protocol/pb/command_move_card.pb.h>
#include <libcockatrice/utility/zone_names.h>
#include <string>

TEST(PlayerActionsTest, PlayCreatureSendsMoveToTable)
{
    GameHarness harness;
    CardState *cat = harness.addCardToZone(ZoneNames::HAND, QStringLiteral("Cat"), 7);
    ASSERT_NE(cat, nullptr);

    harness.player()->getPlayerActions()->playCard(cat, false);

    ASSERT_FALSE(harness.client()->sent.isEmpty());
    const CommandContainer &container = harness.lastCommand();
    ASSERT_EQ(container.game_command_size(), 1);
    const Command_MoveCard command = container.game_command(0).GetExtension(Command_MoveCard::ext);

    EXPECT_EQ(command.start_player_id(), 1);
    EXPECT_EQ(command.start_zone(), std::string(ZoneNames::HAND));
    EXPECT_EQ(command.target_player_id(), 1);
    EXPECT_EQ(command.target_zone(), std::string(ZoneNames::TABLE));
    EXPECT_EQ(command.x(), -1);
    EXPECT_EQ(command.y(), TableZoneLogic::tableRowToGridY(0));

    ASSERT_EQ(command.cards_to_move().card_size(), 1);
    const CardToMove &card = command.cards_to_move().card(0);
    EXPECT_EQ(card.card_id(), 7);
    EXPECT_TRUE(card.has_face_down());
    EXPECT_FALSE(card.face_down());
    EXPECT_TRUE(card.has_pt());
    EXPECT_EQ(card.pt(), "3/3");
    EXPECT_TRUE(card.has_tapped());
    EXPECT_FALSE(card.tapped());
}

TEST(PlayerActionsTest, PlayInstantSendsMoveToStack)
{
    GameHarness harness;
    CardState *bolt = harness.addCardToZone(ZoneNames::HAND, QStringLiteral("Lightning Bolt"), 8);
    ASSERT_NE(bolt, nullptr);

    harness.player()->getPlayerActions()->playCard(bolt, false);

    ASSERT_FALSE(harness.client()->sent.isEmpty());
    const CommandContainer &container = harness.lastCommand();
    ASSERT_EQ(container.game_command_size(), 1);
    const Command_MoveCard command = container.game_command(0).GetExtension(Command_MoveCard::ext);

    EXPECT_EQ(command.target_zone(), std::string(ZoneNames::STACK));
    EXPECT_EQ(command.x(), -1);
    EXPECT_EQ(command.y(), 0);

    ASSERT_EQ(command.cards_to_move().card_size(), 1);
    const CardToMove &card = command.cards_to_move().card(0);
    EXPECT_EQ(card.card_id(), 8);
    // The stack branch deliberately leaves the other attributes unset.
    EXPECT_FALSE(card.has_face_down());
    EXPECT_FALSE(card.has_pt());
    EXPECT_FALSE(card.has_tapped());
}

TEST(PlayerActionsTest, PlayFaceDownCreatureUsesFaceDownRow)
{
    GameHarness harness;
    CardState *cat = harness.addCardToZone(ZoneNames::HAND, QStringLiteral("Cat"), 7);
    ASSERT_NE(cat, nullptr);

    harness.player()->getPlayerActions()->playCard(cat, true);

    ASSERT_FALSE(harness.client()->sent.isEmpty());
    const CommandContainer &container = harness.lastCommand();
    ASSERT_EQ(container.game_command_size(), 1);
    const Command_MoveCard command = container.game_command(0).GetExtension(Command_MoveCard::ext);

    EXPECT_EQ(command.target_zone(), std::string(ZoneNames::TABLE));
    EXPECT_EQ(command.x(), -1);
    EXPECT_EQ(command.y(), TableZoneLogic::tableRowToGridY(2));

    ASSERT_EQ(command.cards_to_move().card_size(), 1);
    const CardToMove &card = command.cards_to_move().card(0);
    EXPECT_EQ(card.card_id(), 7);
    EXPECT_TRUE(card.has_face_down());
    EXPECT_TRUE(card.face_down());
    EXPECT_FALSE(card.has_pt());
    EXPECT_TRUE(card.has_tapped());
    EXPECT_FALSE(card.tapped());
}

TEST(PlayerActionsTest, PlayCiptCreatureSendsTapped)
{
    GameHarness harness;
    CardState *bears = harness.addCardToZone(ZoneNames::HAND, QStringLiteral("Grizzly Bears"), 9);
    ASSERT_NE(bears, nullptr);

    harness.player()->getPlayerActions()->playCard(bears, false);

    ASSERT_FALSE(harness.client()->sent.isEmpty());
    const CommandContainer &container = harness.lastCommand();
    ASSERT_EQ(container.game_command_size(), 1);
    const Command_MoveCard command = container.game_command(0).GetExtension(Command_MoveCard::ext);

    EXPECT_EQ(command.target_zone(), std::string(ZoneNames::TABLE));

    ASSERT_EQ(command.cards_to_move().card_size(), 1);
    const CardToMove &card = command.cards_to_move().card(0);
    EXPECT_EQ(card.card_id(), 9);
    EXPECT_TRUE(card.has_face_down());
    EXPECT_FALSE(card.face_down());
    EXPECT_TRUE(card.has_pt());
    EXPECT_EQ(card.pt(), "2/2");
    EXPECT_TRUE(card.has_tapped());
    EXPECT_TRUE(card.tapped());
}

// These three #7737 command cases use the current shared game harness.
TEST(TokenCommandTest, FaceDownKnownTokenDoesNotRestoreDatabasePowerToughness)
{
    GameHarness harness;
    harness.client()->sent.clear();
    TokenInfo token;
    token.name = QStringLiteral("Kitten");
    token.color = QString(QChar());
    token.faceDown = true;
    harness.player()->getPlayerActions()->actCreateToken(token);
    ASSERT_EQ(harness.client()->sent.size(), 1);
    ASSERT_EQ(harness.lastCommand().game_command_size(), 1);
    const auto &gameCommand = harness.lastCommand().game_command(0);
    ASSERT_TRUE(gameCommand.HasExtension(Command_CreateToken::ext));
    const auto &command = gameCommand.GetExtension(Command_CreateToken::ext);
    EXPECT_EQ(command.card_name(), "Kitten");
    EXPECT_TRUE(command.face_down());
    EXPECT_TRUE(command.pt().empty());
    harness.player()->getPlayerActions()->actCreateAnotherToken();
    ASSERT_EQ(harness.client()->sent.size(), 2);
    const auto &repeat = harness.lastCommand().game_command(0).GetExtension(Command_CreateToken::ext);
    EXPECT_TRUE(repeat.face_down());
    EXPECT_TRUE(repeat.pt().empty());
}

TEST(TokenCommandTest, FaceUpKnownAliasKeepsCanonicalNameAndDefaultPowerToughness)
{
    GameHarness harness;
    harness.client()->sent.clear();
    TokenInfo token;
    token.name = QStringLiteral("kitten");
    harness.player()->getPlayerActions()->actCreateToken(token);
    ASSERT_EQ(harness.client()->sent.size(), 1);
    ASSERT_EQ(harness.lastCommand().game_command_size(), 1);
    const auto &gameCommand = harness.lastCommand().game_command(0);
    ASSERT_TRUE(gameCommand.HasExtension(Command_CreateToken::ext));
    const auto &command = gameCommand.GetExtension(Command_CreateToken::ext);
    EXPECT_EQ(command.card_name(), "Kitten");
    EXPECT_FALSE(command.face_down());
    EXPECT_EQ(command.pt(), "1/1");
}

TEST(TokenCommandTest, FaceUpKnownTokenPreservesExplicitPowerToughness)
{
    GameHarness harness;
    harness.client()->sent.clear();
    TokenInfo token;
    token.name = QStringLiteral("Kitten");
    token.pt = QStringLiteral("7/9");
    token.annotation = QStringLiteral("Flying");
    harness.player()->getPlayerActions()->actCreateToken(token);
    ASSERT_EQ(harness.client()->sent.size(), 1);
    ASSERT_EQ(harness.lastCommand().game_command_size(), 1);
    const auto &gameCommand = harness.lastCommand().game_command(0);
    ASSERT_TRUE(gameCommand.HasExtension(Command_CreateToken::ext));
    const auto &command = gameCommand.GetExtension(Command_CreateToken::ext);
    EXPECT_FALSE(command.face_down());
    EXPECT_EQ(command.pt(), "7/9");
    EXPECT_EQ(command.annotation(), "Flying");
}
