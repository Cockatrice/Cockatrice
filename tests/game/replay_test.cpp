/**
 * @file replay_test.cpp
 *
 * Replays a synthetic GameReplay through the real dispatcher. This is the only
 * place that exercises PlayerLogic::processPlayerInfo / processCardAttachment
 * end to end: the game state arrives as a GAME_STATE_CHANGED event and the
 * graphics side is replaced by a small materializer that mirrors what
 * CardZone::onCreateCardRequested does with the logic objects.
 *
 * The replay is built in code rather than from a .cor file: event application
 * is deterministic (the client never shuffles on its own for a replay), and a
 * checked-in binary fixture would be opaque about what it asserts.
 */

#include "gtest/gtest.h"
#include <QList>
#include <QObject>
#include <QString>
#include <QtAlgorithms>
#include <libcockatrice/game/board/card_state.h>
#include <libcockatrice/game/game_event_handler.h>
#include <libcockatrice/game/game_state.h>
#include <libcockatrice/game/player/event_processing_options.h>
#include <libcockatrice/game/player/player_logic.h>
#include <libcockatrice/game/player/player_manager.h>
#include <libcockatrice/game/replay.h>
#include <libcockatrice/game/zones/card_zone_logic.h>
#include <libcockatrice/game/zones/table_zone_logic.h>
#include <libcockatrice/protocol/pb/event_game_state_changed.pb.h>
#include <libcockatrice/protocol/pb/event_move_card.pb.h>
#include <libcockatrice/protocol/pb/event_set_card_attr.pb.h>
#include <libcockatrice/protocol/pb/event_set_card_counter.pb.h>
#include <libcockatrice/protocol/pb/game_event_container.pb.h>
#include <libcockatrice/protocol/pb/game_replay.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_card.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_player.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_user.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_zone.pb.h>
#include <libcockatrice/utility/card_ref.h>
#include <libcockatrice/utility/zone_names.h>
#include <memory>
#include <string>
#include <utility>

namespace
{

ServerInfo_Card makeCard(int id, const QString &name, int x = -1, int y = -1)
{
    ServerInfo_Card card;
    card.set_id(id);
    card.set_name(name.toStdString());
    card.set_x(x);
    card.set_y(y);
    return card;
}

ServerInfo_Zone makeZone(const char *name,
                         ServerInfo_Zone::ZoneType type,
                         bool withCoords,
                         const QList<ServerInfo_Card> &cards,
                         int cardCount = 0)
{
    ServerInfo_Zone zone;
    zone.set_name(name);
    zone.set_type(type);
    zone.set_with_coords(withCoords);
    for (const auto &card : cards) {
        zone.add_card_list()->CopyFrom(card);
    }
    if (cardCount > 0) {
        zone.set_card_count(cardCount);
    }
    return zone;
}

ServerInfo_Player makePlayer(int playerId, const QString &name, const QList<ServerInfo_Zone> &zones)
{
    ServerInfo_Player player;
    ServerInfo_PlayerProperties *properties = player.mutable_properties();
    properties->set_player_id(playerId);
    properties->mutable_user_info()->set_name(name.toStdString());
    properties->mutable_user_info()->set_user_level(ServerInfo_User::IsUser);
    for (const auto &zone : zones) {
        player.add_zone_list()->CopyFrom(zone);
    }
    return player;
}

template <typename EventType> GameEventContainer makeContainer(int playerId, const EventType &message)
{
    GameEventContainer container;
    container.set_game_id(42);

    GameEvent *event = container.add_event_list();
    event->set_player_id(playerId);
    event->MutableExtension(EventType::ext)->CopyFrom(message);

    return container;
}

/**
 * Stands in for the graphics side of a replay: when the logic asks for a card
 * item, create the CardState and add it to the zone, copying the state fields
 * the real CardItem::processCardInfo applies.
 *
 * The production view owns the visual item; the state itself is all a headless
 * test needs, and this is the smallest thing that lets processPlayerInfo build
 * a real zone.
 */
class HeadlessZoneView : public QObject
{
public:
    using QObject::QObject;

    ~HeadlessZoneView() override
    {
        qDeleteAll(cards_);
        cards_.clear();
    }

    void attach(PlayerLogic *player)
    {
        for (CardZoneLogic *zone : player->getZones()) {
            connect(zone, &CardZoneLogic::createCardRequested, this,
                    [this, zone](const ServerInfo_Card &cardInfo, bool reorganize) {
                        materialize(zone, cardInfo, reorganize);
                    });
        }
    }

private:
    void materialize(CardZoneLogic *zone, const ServerInfo_Card &info, bool reorganize)
    {
        const CardRef cardRef{QString::fromStdString(info.name()), QString::fromStdString(info.provider_id())};
        auto *card = new CardState(nullptr, zone, cardRef, info.id());
        // The real CardItem constructor refreshes the card info before
        // processCardInfo overwrites the state fields.
        card->refreshCardInfo();

        card->clearCounters();
        for (int i = 0; i < info.counter_list_size(); ++i) {
            card->insertCounter(info.counter_list(i).id(), info.counter_list(i).value());
        }
        card->setAttacking(info.attacking());
        card->setFaceDown(info.face_down());
        card->setPT(QString::fromStdString(info.pt()));
        card->setAnnotation(QString::fromStdString(info.annotation()));
        card->setColor(QString::fromStdString(info.color()));
        card->setTapped(info.tapped());
        card->setDestroyOnZoneChange(info.destroy_on_zone_change());
        card->setDoesntUntap(info.doesnt_untap());

        zone->addCard(card, reorganize, info.x(), info.y());
        cards_.append(card);
    }

    QList<CardState *> cards_;
};

/** Owns a Replay built from a GameReplay and feeds its containers through it. */
class ReplayHarness
{
public:
    explicit ReplayHarness(GameReplay replay)
        : replayProto_(std::move(replay)), view_(std::make_unique<HeadlessZoneView>()),
          replay_(std::make_unique<Replay>(nullptr, &replayProto_, false))
    {
        QObject::connect(replay_->getPlayerManager(), &PlayerManager::playerAdded, view_.get(),
                         &HeadlessZoneView::attach);
    }

    ReplayHarness(const ReplayHarness &) = delete;
    ReplayHarness &operator=(const ReplayHarness &) = delete;

    void processAll()
    {
        for (const GameEventContainer &container : replayProto_.event_list()) {
            replay_->getGameEventHandler()->processGameEventContainer(container, nullptr, EventProcessingOptions());
        }
    }

    [[nodiscard]] Replay *replay() const
    {
        return replay_.get();
    }

    [[nodiscard]] PlayerLogic *player(int playerId) const
    {
        return replay_->getPlayerManager()->getPlayer(playerId);
    }

private:
    GameReplay replayProto_;
    std::unique_ptr<HeadlessZoneView> view_;
    std::unique_ptr<Replay> replay_;
};

GameReplay makeReplay(const QList<GameEventContainer> &containers)
{
    GameReplay replay;
    replay.mutable_game_info()->set_game_id(42);
    replay.mutable_game_info()->set_started(true);
    for (const auto &container : containers) {
        replay.add_event_list()->CopyFrom(container);
    }
    return replay;
}

Event_GameStateChanged initialGameState()
{
    Event_GameStateChanged state;
    state.set_game_started(true);
    state.set_active_player_id(1);
    state.set_active_phase(0);
    return state;
}

} // namespace

TEST(ReplayTest, MaterializesGameStateZonesAndAttachments)
{
    Event_GameStateChanged state = initialGameState();

    ServerInfo_Card cat = makeCard(7, QStringLiteral("Cat"));
    // Cross-player attachment: Alice's card is attached to Bob's creature.
    cat.set_attach_player_id(2);
    cat.set_attach_zone(ZoneNames::TABLE);
    cat.set_attach_card_id(9);

    const ServerInfo_Player alice = makePlayer(1, QStringLiteral("Alice"),
                                               {makeZone(ZoneNames::HAND, ServerInfo_Zone::PrivateZone, false, {cat}),
                                                makeZone(ZoneNames::TABLE, ServerInfo_Zone::PublicZone, true,
                                                         {makeCard(9, QStringLiteral("Grizzly Bears"), 2, 1)}),
                                                makeZone(ZoneNames::DECK, ServerInfo_Zone::HiddenZone, false, {}, 2)});
    const ServerInfo_Player bob = makePlayer(2, QStringLiteral("Bob"),
                                             {makeZone(ZoneNames::TABLE, ServerInfo_Zone::PublicZone, true,
                                                       {makeCard(9, QStringLiteral("Grizzly Bears"), 1, 0)})});

    state.add_player_list()->CopyFrom(alice);
    state.add_player_list()->CopyFrom(bob);

    ReplayHarness harness(makeReplay({makeContainer(-1, state)}));
    harness.processAll();

    PlayerLogic *aliceLogic = harness.player(1);
    PlayerLogic *bobLogic = harness.player(2);
    ASSERT_NE(aliceLogic, nullptr);
    ASSERT_NE(bobLogic, nullptr);

    CardZoneLogic *hand = aliceLogic->getZones().value(ZoneNames::HAND);
    CardZoneLogic *table = aliceLogic->getZones().value(ZoneNames::TABLE);
    CardZoneLogic *deck = aliceLogic->getZones().value(ZoneNames::DECK);
    ASSERT_NE(hand, nullptr);
    ASSERT_NE(table, nullptr);
    ASSERT_NE(deck, nullptr);

    ASSERT_EQ(hand->getCards().size(), 1);
    CardState *catState = hand->getCards().first();
    EXPECT_EQ(catState->getId(), 7);
    EXPECT_EQ(catState->getName(), QStringLiteral("Cat"));

    ASSERT_EQ(table->getCards().size(), 1);
    CardState *host = table->getCards().first();
    EXPECT_EQ(host->getName(), QStringLiteral("Grizzly Bears"));
    EXPECT_EQ(host->getGridPoint(), QPoint(2, 1));

    // The hidden deck only carries a count, and the view materializes that many
    // unknown cards.
    EXPECT_EQ(deck->getCards().size(), 2);
    for (const CardState *hidden : deck->getCards()) {
        EXPECT_EQ(hidden->getId(), -1);
    }

    // processCardAttachment links both sides after every player is materialized.
    CardZoneLogic *bobTable = bobLogic->getZones().value(ZoneNames::TABLE);
    ASSERT_NE(bobTable, nullptr);
    ASSERT_EQ(bobTable->getCards().size(), 1);
    CardState *bobHost = bobTable->getCards().first();
    EXPECT_EQ(catState->getAttachedTo(), bobHost);
    ASSERT_EQ(bobHost->getAttachedCards().size(), 1);
    EXPECT_EQ(bobHost->getAttachedCards().first(), catState);
    // The identically numbered card on Alice's own table is a different card.
    EXPECT_TRUE(host->getAttachedCards().isEmpty());

    EXPECT_TRUE(harness.replay()->getGameState()->isGameStateKnown());
}

TEST(ReplayTest, AppliesMovesAttributesAndCountersInOrder)
{
    Event_GameStateChanged state = initialGameState();
    state.add_player_list()->CopyFrom(makePlayer(
        1, QStringLiteral("Alice"),
        {makeZone(ZoneNames::HAND, ServerInfo_Zone::PrivateZone, false, {makeCard(7, QStringLiteral("Cat"))}),
         makeZone(ZoneNames::TABLE, ServerInfo_Zone::PublicZone, true,
                  {makeCard(9, QStringLiteral("Grizzly Bears"), 0, TableZoneLogic::tableRowToGridY(0))}),
         makeZone(ZoneNames::DECK, ServerInfo_Zone::HiddenZone, false, {}, 2)}));

    Event_MoveCard move;
    move.set_card_id(7);
    move.set_start_player_id(1);
    move.set_start_zone(ZoneNames::HAND);
    move.set_position(0);
    move.set_target_player_id(1);
    move.set_target_zone(ZoneNames::TABLE);
    move.set_x(3);
    move.set_y(TableZoneLogic::tableRowToGridY(1));
    move.set_new_card_id(7);
    move.set_face_down(false);

    Event_SetCardAttr tap;
    tap.set_zone_name(ZoneNames::TABLE);
    tap.set_card_id(7);
    tap.set_attribute(AttrTapped);
    tap.set_attr_value("1");

    Event_SetCardCounter counter;
    counter.set_zone_name(ZoneNames::TABLE);
    counter.set_card_id(7);
    counter.set_counter_id(1);
    counter.set_counter_value(2);

    ReplayHarness harness(makeReplay(
        {makeContainer(-1, state), makeContainer(1, move), makeContainer(1, tap), makeContainer(1, counter)}));
    harness.processAll();

    PlayerLogic *alice = harness.player(1);
    ASSERT_NE(alice, nullptr);
    CardZoneLogic *hand = alice->getZones().value(ZoneNames::HAND);
    CardZoneLogic *table = alice->getZones().value(ZoneNames::TABLE);
    ASSERT_NE(hand, nullptr);
    ASSERT_NE(table, nullptr);

    EXPECT_TRUE(hand->getCards().isEmpty());
    ASSERT_EQ(table->getCards().size(), 2);

    CardState *moved = table->getCard(7);
    ASSERT_NE(moved, nullptr);
    EXPECT_EQ(moved->getGridPoint(), QPoint(3, TableZoneLogic::tableRowToGridY(1)));
    EXPECT_EQ(moved->getPT(), QStringLiteral("3/3"));
    EXPECT_TRUE(moved->getTapped());
    EXPECT_EQ(moved->getCounters().value(1), 2);

    // The untouched creature keeps its own position and state.
    CardState *bears = table->getCard(9);
    ASSERT_NE(bears, nullptr);
    EXPECT_EQ(bears->getGridPoint(), QPoint(0, TableZoneLogic::tableRowToGridY(0)));
    EXPECT_FALSE(bears->getTapped());
}
