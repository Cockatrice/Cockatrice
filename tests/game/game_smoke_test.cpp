/**
 * @file game_smoke_test.cpp
 *
 * Links the game logic library into a target that contains no graphics code at all, and drives
 * the two seams the graphics layer used to own: the attachment graph on CardState and the
 * card-creation request a zone makes so the graphics side can build the matching card item.
 */

#include "gtest/gtest.h"
#include <QCoreApplication>
#include <QList>
#include <QLoggingCategory>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <libcockatrice/card/card_info.h>
#include <libcockatrice/card/database/card_database.h>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/game/abstract_game.h>
#include <libcockatrice/game/board/card_state.h>
#include <libcockatrice/game/game_state.h>
#include <libcockatrice/game/player/player_actions.h>
#include <libcockatrice/game/player/player_logic.h>
#include <libcockatrice/game/player/player_manager.h>
#include <libcockatrice/game/zones/hand_zone_logic.h>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/pb/command_create_token.pb.h>
#include <libcockatrice/protocol/pb/commands.pb.h>
#include <libcockatrice/protocol/pending_command.h>

namespace
{

CardRef cardRefFor(const QString &name)
{
    CardRef cardRef;
    cardRef.name = name;
    return cardRef;
}

// Capture the real serialized command at the transport boundary. PlayerActions,
// GameEventHandler and PendingCommand retain their production behavior.
class CapturingClient : public AbstractClient
{
public:
    QList<CommandContainer> commands;

    ~CapturingClient() override
    {
        for (auto *command : pendingCommands) {
            delete command;
        }
    }

protected:
    void sendCommandContainer(const CommandContainer &command) override
    {
        commands.append(command);
    }
};

class TokenCommandTest : public ::testing::Test
{
protected:
    // The client outlives the game and its pending-command signal receivers.
    CapturingClient client;
    AbstractGame game{nullptr};
    PlayerLogic *player = nullptr;
    CardInfoPtr knownToken;

    void SetUp() override
    {
        knownToken = CardInfo::newInstance(QStringLiteral("Token Command Test Goblin"), "", true, {}, {}, {}, {},
                                           {.tableRow = 2});
        knownToken->setColors(QStringLiteral("R"));
        knownToken->setPowTough(QStringLiteral("2/2"));
        CardDatabaseManager::getInstance()->addCard(knownToken);

        game.gameState = new GameState(&game, 0, 0, false, {&client}, true, false, 0, false);
        game.playerManager = new PlayerManager(&game, 0, false, false);
        ServerInfo_User user;
        user.set_name("Token command test player");
        player = game.playerManager->addPlayer(0, user);
    }

    void TearDown() override
    {
        CardDatabaseManager::getInstance()->removeCard(knownToken);
    }
};

} // namespace

TEST(CardStateTest, AttachmentGraphKeepsBothEndsConsistent)
{
    CardState host(nullptr, nullptr, cardRefFor(QStringLiteral("Host")), 1);
    CardState guest(nullptr, nullptr, cardRefFor(QStringLiteral("Guest")), 2);

    guest.setAttachedTo(&host);
    host.addAttachedCard(&guest);

    EXPECT_EQ(guest.getAttachedTo(), &host);
    ASSERT_EQ(host.getAttachedCards().size(), 1);
    EXPECT_EQ(host.getAttachedCards().first(), &guest);

    host.removeAttachedCard(&guest);
    guest.setAttachedTo(nullptr);

    EXPECT_TRUE(host.getAttachedCards().isEmpty());
    EXPECT_EQ(guest.getAttachedTo(), nullptr);
}

TEST(CardZoneLogicTest, AddsAndTakesCardsWithoutAGraphicsSide)
{
    HandZoneLogic zone(nullptr, QStringLiteral("hand"), false, false, true);
    CardState card(nullptr, nullptr, cardRefFor(QStringLiteral("Lightning Bolt")), 7);

    QSignalSpy cardAddedSpy(&zone, &CardZoneLogic::cardAdded);
    QSignalSpy cardCountSpy(&zone, &CardZoneLogic::cardCountChanged);

    zone.addCard(&card, false, 0);

    ASSERT_EQ(zone.getCards().size(), 1);
    EXPECT_EQ(zone.getCards().first(), &card);
    EXPECT_EQ(card.getZone(), &zone);
    EXPECT_EQ(cardAddedSpy.count(), 1);
    EXPECT_GE(cardCountSpy.count(), 1);

    EXPECT_EQ(zone.takeCard(0, 7), &card);
    EXPECT_TRUE(zone.getCards().isEmpty());
    EXPECT_EQ(card.getId(), 7);
}

TEST(CardZoneLogicTest, AsksTheGraphicsSideToCreateCards)
{
    HandZoneLogic zone(nullptr, QStringLiteral("library"), false, true, true);
    QSignalSpy requestSpy(&zone, &CardZoneLogic::createCardRequested);

    ServerInfo_Card cardInfo;
    cardInfo.set_name("Lightning Bolt");
    cardInfo.set_id(7);
    cardInfo.set_x(0);
    cardInfo.set_y(1);

    zone.createCardRequested(cardInfo, true);

    ASSERT_EQ(requestSpy.count(), 1);
    const QList<QVariant> arguments = requestSpy.takeFirst();
    EXPECT_EQ(arguments.at(0).value<ServerInfo_Card>().id(), 7);
    EXPECT_TRUE(arguments.at(1).toBool());
}

TEST_F(TokenCommandTest, FaceDownKnownTokenDoesNotRestoreDatabasePowerToughness)
{
    TokenInfo token;
    token.name = knownToken->getName();
    token.color = QString(QChar());
    token.faceDown = true;
    player->getPlayerActions()->actCreateToken(token);

    ASSERT_EQ(client.commands.size(), 1);
    ASSERT_EQ(client.commands.first().game_command_size(), 1);
    const auto &gameCommand = client.commands.first().game_command(0);
    ASSERT_TRUE(gameCommand.HasExtension(Command_CreateToken::ext));
    const auto &command = gameCommand.GetExtension(Command_CreateToken::ext);
    EXPECT_EQ(command.card_name(), knownToken->getName().toStdString());
    EXPECT_TRUE(command.face_down());
    EXPECT_TRUE(command.pt().empty());

    player->getPlayerActions()->actCreateAnotherToken();
    ASSERT_EQ(client.commands.size(), 2);
    const auto &repeat = client.commands.last().game_command(0).GetExtension(Command_CreateToken::ext);
    EXPECT_TRUE(repeat.face_down());
    EXPECT_TRUE(repeat.pt().empty());
}

TEST_F(TokenCommandTest, FaceUpKnownAliasKeepsCanonicalNameAndDefaultPowerToughness)
{
    TokenInfo token;
    token.name = QStringLiteral("token command test goblin");
    player->getPlayerActions()->actCreateToken(token);

    ASSERT_EQ(client.commands.size(), 1);
    ASSERT_EQ(client.commands.first().game_command_size(), 1);
    const auto &gameCommand = client.commands.first().game_command(0);
    ASSERT_TRUE(gameCommand.HasExtension(Command_CreateToken::ext));
    const auto &command = gameCommand.GetExtension(Command_CreateToken::ext);
    EXPECT_EQ(command.card_name(), knownToken->getName().toStdString());
    EXPECT_FALSE(command.face_down());
    EXPECT_EQ(command.pt(), "2/2");
}

TEST_F(TokenCommandTest, FaceUpKnownTokenPreservesExplicitPowerToughness)
{
    TokenInfo token;
    token.name = knownToken->getName();
    token.pt = QStringLiteral("7/9");
    token.annotation = QStringLiteral("Flying");
    player->getPlayerActions()->actCreateToken(token);

    ASSERT_EQ(client.commands.size(), 1);
    ASSERT_EQ(client.commands.first().game_command_size(), 1);
    const auto &gameCommand = client.commands.first().game_command(0);
    ASSERT_TRUE(gameCommand.HasExtension(Command_CreateToken::ext));
    const auto &command = gameCommand.GetExtension(Command_CreateToken::ext);
    EXPECT_FALSE(command.face_down());
    EXPECT_EQ(command.pt(), "7/9");
    EXPECT_EQ(command.annotation(), "Flying");
}

int main(int argc, char **argv)
{
    // Keep SettingsCache reads and writes away from the real user profile: the game logic still
    // reaches for a handful of settings, and the client settings sources linked into this test
    // would otherwise touch the developer's own configuration.
    QStandardPaths::setTestModeEnabled(true);

    QTemporaryDir home;
    if (home.isValid()) {
        qputenv("HOME", home.path().toLocal8Bit());
    }

    QCoreApplication app(argc, argv);
    QLoggingCategory::setFilterRules("settings_cache.*=false");

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
