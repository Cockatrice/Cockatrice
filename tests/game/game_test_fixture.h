/**
 * @file game_test_fixture.h
 *
 * Fixtures for tests that need a complete game/player/zone graph without the
 * client, a server or any graphics object. Commands are captured on a recording
 * AbstractClient, which is the same seam the real client uses.
 */

#ifndef COCKATRICE_TESTS_GAME_TEST_FIXTURE_H
#define COCKATRICE_TESTS_GAME_TEST_FIXTURE_H

#include <QList>
#include <QMap>
#include <QString>
#include <QtAlgorithms>
#include <libcockatrice/game/board/card_state.h>
#include <libcockatrice/game/game.h>
#include <libcockatrice/game/player/player_logic.h>
#include <libcockatrice/game/player/player_manager.h>
#include <libcockatrice/game/zones/card_zone_logic.h>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/pb/commands.pb.h>
#include <libcockatrice/protocol/pb/event_game_joined.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_user.pb.h>
#include <libcockatrice/utility/card_ref.h>
#include <memory>
#include <utility>

/**
 * Captures every command the game logic sends, instead of writing it to a socket.
 *
 * AbstractClient::sendCommand uses a direct signal/slot connection in the same
 * thread, so a command is in sent as soon as sendCommand returns.
 */
class RecordingClient : public AbstractClient
{
public:
    QList<CommandContainer> sent;

protected:
    void sendCommandContainer(const CommandContainer &cont) override
    {
        sent.append(cont);
    }
};

/**
 * Owns a local, single-client Game with one local player, plus the cards added
 * to its zones.
 */
class GameHarness
{
public:
    explicit GameHarness(int playerId = 1) : playerId_(playerId), client_(std::make_unique<RecordingClient>())
    {
        Event_GameJoined event;
        event.set_host_id(playerId_);
        event.set_player_id(playerId_);
        event.mutable_game_info()->set_game_id(42);
        event.mutable_game_info()->set_started(true);

        QList<AbstractClient *> clients{client_.get()};
        QMap<int, QString> roomGameTypes;
        game_ = std::make_unique<Game>(nullptr, true, clients, event, roomGameTypes);

        ServerInfo_User user;
        user.set_name("Alice");
        user.set_user_level(ServerInfo_User::IsUser | ServerInfo_User::IsRegistered);
        game_->getPlayerManager()->addPlayer(playerId_, user);
    }

    ~GameHarness()
    {
        qDeleteAll(ownedCards_);
        ownedCards_.clear();
    }

    GameHarness(const GameHarness &) = delete;
    GameHarness &operator=(const GameHarness &) = delete;

    [[nodiscard]] Game *game() const
    {
        return game_.get();
    }

    [[nodiscard]] RecordingClient *client() const
    {
        return client_.get();
    }

    [[nodiscard]] PlayerLogic *player() const
    {
        return game_->getPlayerManager()->getPlayer(playerId_);
    }

    [[nodiscard]] CardZoneLogic *zone(const QString &zoneName) const
    {
        return player()->getZones().value(zoneName);
    }

    CardState *addCardToZone(const QString &zoneName, const QString &cardName, int cardId)
    {
        CardZoneLogic *targetZone = zone(zoneName);
        if (!targetZone) {
            return nullptr;
        }

        auto *card = new CardState(nullptr, targetZone, CardRef{cardName, QString()}, cardId);
        // The constructor only stores the ref; this is what fills in the card
        // data (table row, P/T, ...) the actions read.
        card->refreshCardInfo();
        targetZone->addCard(card, false, 0);
        ownedCards_.append(card);
        return card;
    }

    [[nodiscard]] const CommandContainer &lastCommand() const
    {
        return client_->sent.last();
    }

private:
    int playerId_;
    std::unique_ptr<RecordingClient> client_;
    std::unique_ptr<Game> game_;
    QList<CardState *> ownedCards_;
};

#endif // COCKATRICE_TESTS_GAME_TEST_FIXTURE_H
