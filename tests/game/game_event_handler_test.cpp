#include "game_test_fixture.h"

#include "gtest/gtest.h"
#include <QList>
#include <QObject>
#include <QPoint>
#include <QString>
#include <libcockatrice/game/board/card_state.h>
#include <libcockatrice/game/board/counter_state.h>
#include <libcockatrice/game/game_event_handler.h>
#include <libcockatrice/game/game_meta_info.h>
#include <libcockatrice/game/game_state.h>
#include <libcockatrice/game/zones/table_zone_logic.h>
#include <libcockatrice/protocol/pb/context_concede.pb.h>
#include <libcockatrice/protocol/pb/event_attach_card.pb.h>
#include <libcockatrice/protocol/pb/event_change_zone_properties.pb.h>
#include <libcockatrice/protocol/pb/event_create_counter.pb.h>
#include <libcockatrice/protocol/pb/event_create_token.pb.h>
#include <libcockatrice/protocol/pb/event_del_counter.pb.h>
#include <libcockatrice/protocol/pb/event_destroy_card.pb.h>
#include <libcockatrice/protocol/pb/event_draw_cards.pb.h>
#include <libcockatrice/protocol/pb/event_flip_card.pb.h>
#include <libcockatrice/protocol/pb/event_game_closed.pb.h>
#include <libcockatrice/protocol/pb/event_game_host_changed.pb.h>
#include <libcockatrice/protocol/pb/event_game_state_changed.pb.h>
#include <libcockatrice/protocol/pb/event_kicked.pb.h>
#include <libcockatrice/protocol/pb/event_leave.pb.h>
#include <libcockatrice/protocol/pb/event_move_card.pb.h>
#include <libcockatrice/protocol/pb/event_player_properties_changed.pb.h>
#include <libcockatrice/protocol/pb/event_set_active_phase.pb.h>
#include <libcockatrice/protocol/pb/event_set_active_player.pb.h>
#include <libcockatrice/protocol/pb/event_set_card_attr.pb.h>
#include <libcockatrice/protocol/pb/event_set_card_counter.pb.h>
#include <libcockatrice/protocol/pb/event_set_counter.pb.h>
#include <libcockatrice/protocol/pb/game_event_container.pb.h>
#include <libcockatrice/protocol/pb/game_event_context.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_card.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_counter.pb.h>
#include <libcockatrice/utility/zone_names.h>
#include <string>

namespace
{

/**
 * Wraps one player-scoped event in the container the server would send and
 * pushes it through the real dispatcher, so the test exercises the same
 * GameEventHandler -> PlayerEventHandler path as a live client.
 */
template <typename EventType>
void sendGameEvent(GameHarness &harness,
                   int playerId,
                   const EventType &message,
                   const GameEventContext *context = nullptr)
{
    GameEventContainer container;
    container.set_game_id(42);
    if (context != nullptr) {
        container.mutable_context()->CopyFrom(*context);
    }

    GameEvent *event = container.add_event_list();
    event->set_player_id(playerId);
    event->MutableExtension(EventType::ext)->CopyFrom(message);

    harness.game()->getGameEventHandler()->processGameEventContainer(container, harness.client(),
                                                                     EventProcessingOptions());
}

Event_SetCardAttr cardAttrEvent(const char *zoneName, int cardId, CardAttribute attribute, const char *value)
{
    Event_SetCardAttr event;
    event.set_zone_name(zoneName);
    event.set_card_id(cardId);
    event.set_attribute(attribute);
    event.set_attr_value(value);
    return event;
}

} // namespace

TEST(GameEventHandlerTest, MoveCardMovesBetweenZones)
{
    GameHarness harness;
    CardState *cat = harness.addCardToZone(ZoneNames::HAND, QStringLiteral("Cat"), 7);
    ASSERT_NE(cat, nullptr);

    Event_MoveCard event;
    event.set_card_id(7);
    event.set_start_player_id(1);
    event.set_start_zone(ZoneNames::HAND);
    event.set_position(0);
    event.set_target_player_id(1);
    event.set_target_zone(ZoneNames::TABLE);
    event.set_x(0);
    event.set_y(TableZoneLogic::tableRowToGridY(0));
    event.set_new_card_id(7);
    event.set_face_down(false);

    sendGameEvent(harness, 1, event);

    EXPECT_TRUE(harness.zone(ZoneNames::HAND)->getCards().isEmpty());
    CardZoneLogic *table = harness.zone(ZoneNames::TABLE);
    ASSERT_EQ(table->getCards().size(), 1);
    EXPECT_EQ(table->getCards().first(), cat);
    EXPECT_EQ(cat->getZone(), table);
    EXPECT_EQ(cat->getId(), 7);
    EXPECT_EQ(cat->getCardRef().name, QStringLiteral("Cat"));
    EXPECT_FALSE(cat->getFaceDown());
    EXPECT_EQ(cat->getPT(), QStringLiteral("3/3"));
    EXPECT_EQ(cat->getGridPoint(), QPoint(0, TableZoneLogic::tableRowToGridY(0)));
}

TEST(GameEventHandlerTest, MoveCardFaceDownKeepsTheCardHidden)
{
    GameHarness harness;
    CardState *cat = harness.addCardToZone(ZoneNames::HAND, QStringLiteral("Cat"), 7);
    ASSERT_NE(cat, nullptr);

    Event_MoveCard event;
    event.set_card_id(7);
    event.set_start_player_id(1);
    event.set_start_zone(ZoneNames::HAND);
    event.set_position(0);
    event.set_target_player_id(1);
    event.set_target_zone(ZoneNames::TABLE);
    event.set_x(1);
    event.set_y(TableZoneLogic::tableRowToGridY(2));
    event.set_new_card_id(7);
    event.set_face_down(true);

    sendGameEvent(harness, 1, event);

    ASSERT_EQ(harness.zone(ZoneNames::TABLE)->getCards().size(), 1);
    EXPECT_TRUE(cat->getFaceDown());
    // A face-down card keeps whatever P/T it had instead of picking up the
    // printed one from the card database.
    EXPECT_TRUE(cat->getPT().isEmpty());
}

TEST(GameEventHandlerTest, SetCardAttrUpdatesSingleCard)
{
    GameHarness harness;
    CardState *cat = harness.addCardToZone(ZoneNames::TABLE, QStringLiteral("Cat"), 7);
    ASSERT_NE(cat, nullptr);

    sendGameEvent(harness, 1, cardAttrEvent(ZoneNames::TABLE, 7, AttrTapped, "1"));
    sendGameEvent(harness, 1, cardAttrEvent(ZoneNames::TABLE, 7, AttrAttacking, "1"));
    sendGameEvent(harness, 1, cardAttrEvent(ZoneNames::TABLE, 7, AttrPT, "5/5"));
    sendGameEvent(harness, 1, cardAttrEvent(ZoneNames::TABLE, 7, AttrAnnotation, "note"));
    sendGameEvent(harness, 1, cardAttrEvent(ZoneNames::TABLE, 7, AttrColor, "red"));
    sendGameEvent(harness, 1, cardAttrEvent(ZoneNames::TABLE, 7, AttrDoesntUntap, "1"));
    sendGameEvent(harness, 1, cardAttrEvent(ZoneNames::TABLE, 7, AttrFaceDown, "1"));

    EXPECT_TRUE(cat->getTapped());
    EXPECT_TRUE(cat->getAttacking());
    EXPECT_EQ(cat->getPT(), QStringLiteral("5/5"));
    EXPECT_EQ(cat->getAnnotation(), QStringLiteral("note"));
    EXPECT_EQ(cat->getColor(), QStringLiteral("red"));
    EXPECT_TRUE(cat->getDoesntUntap());
    EXPECT_TRUE(cat->getFaceDown());
}

TEST(GameEventHandlerTest, SetCardAttrWithoutCardIdTouchesEveryCardInZone)
{
    GameHarness harness;
    CardState *cat = harness.addCardToZone(ZoneNames::TABLE, QStringLiteral("Cat"), 7);
    CardState *bears = harness.addCardToZone(ZoneNames::TABLE, QStringLiteral("Grizzly Bears"), 9);
    ASSERT_NE(cat, nullptr);
    ASSERT_NE(bears, nullptr);

    Event_SetCardAttr event;
    event.set_zone_name(ZoneNames::TABLE);
    event.set_attribute(AttrAnnotation);
    event.set_attr_value("marked");

    sendGameEvent(harness, 1, event);

    EXPECT_EQ(cat->getAnnotation(), QStringLiteral("marked"));
    EXPECT_EQ(bears->getAnnotation(), QStringLiteral("marked"));
}

TEST(GameEventHandlerTest, UntapAllLeavesDoesntUntapCardsTapped)
{
    GameHarness harness;
    CardState *cat = harness.addCardToZone(ZoneNames::TABLE, QStringLiteral("Cat"), 7);
    CardState *bears = harness.addCardToZone(ZoneNames::TABLE, QStringLiteral("Grizzly Bears"), 9);
    ASSERT_NE(cat, nullptr);
    ASSERT_NE(bears, nullptr);
    cat->setDoesntUntap(true);

    sendGameEvent(harness, 1, cardAttrEvent(ZoneNames::TABLE, 7, AttrTapped, "1"));
    sendGameEvent(harness, 1, cardAttrEvent(ZoneNames::TABLE, 9, AttrTapped, "1"));

    Event_SetCardAttr untapAll;
    untapAll.set_zone_name(ZoneNames::TABLE);
    untapAll.set_attribute(AttrTapped);
    untapAll.set_attr_value("0");

    sendGameEvent(harness, 1, untapAll);

    EXPECT_TRUE(cat->getTapped());
    EXPECT_FALSE(bears->getTapped());
}

TEST(GameEventHandlerTest, SetCardCounterAddsAndRemoves)
{
    GameHarness harness;
    CardState *cat = harness.addCardToZone(ZoneNames::HAND, QStringLiteral("Cat"), 7);
    ASSERT_NE(cat, nullptr);

    Event_SetCardCounter add;
    add.set_zone_name(ZoneNames::HAND);
    add.set_card_id(7);
    add.set_counter_id(1);
    add.set_counter_value(2);

    sendGameEvent(harness, 1, add);

    EXPECT_EQ(cat->getCounters().value(1), 2);

    Event_SetCardCounter remove = add;
    remove.set_counter_value(0);

    sendGameEvent(harness, 1, remove);

    EXPECT_FALSE(cat->getCounters().contains(1));
}

TEST(GameEventHandlerTest, AttachCardLinksBothEndsAndUnattachClearsThem)
{
    GameHarness harness;
    CardState *host = harness.addCardToZone(ZoneNames::TABLE, QStringLiteral("Cat"), 7);
    CardState *guest = harness.addCardToZone(ZoneNames::TABLE, QStringLiteral("Grizzly Bears"), 9);
    ASSERT_NE(host, nullptr);
    ASSERT_NE(guest, nullptr);

    Event_AttachCard attach;
    attach.set_start_zone(ZoneNames::TABLE);
    attach.set_card_id(9);
    attach.set_target_player_id(1);
    attach.set_target_zone(ZoneNames::TABLE);
    attach.set_target_card_id(7);

    sendGameEvent(harness, 1, attach);

    EXPECT_EQ(guest->getAttachedTo(), host);
    ASSERT_EQ(host->getAttachedCards().size(), 1);
    EXPECT_EQ(host->getAttachedCards().first(), guest);

    Event_AttachCard detach;
    detach.set_start_zone(ZoneNames::TABLE);
    detach.set_card_id(9);

    sendGameEvent(harness, 1, detach);

    EXPECT_EQ(guest->getAttachedTo(), nullptr);
    EXPECT_TRUE(host->getAttachedCards().isEmpty());
}

TEST(GameEventHandlerTest, ChangeZonePropertiesUpdatesReveal)
{
    GameHarness harness;
    CardZoneLogic *deck = harness.zone(ZoneNames::DECK);
    ASSERT_NE(deck, nullptr);

    Event_ChangeZoneProperties reveal;
    reveal.set_zone_name(ZoneNames::DECK);
    reveal.set_always_reveal_top_card(true);

    sendGameEvent(harness, 1, reveal);
    EXPECT_TRUE(deck->getAlwaysRevealTopCard());

    reveal.set_always_reveal_top_card(false);
    sendGameEvent(harness, 1, reveal);
    EXPECT_FALSE(deck->getAlwaysRevealTopCard());
}

TEST(GameEventHandlerTest, DrawCardsMovesNamedCardsFromDeckToHand)
{
    GameHarness harness;
    ASSERT_NE(harness.addCardToZone(ZoneNames::DECK, QStringLiteral("Cat"), 7), nullptr);

    Event_DrawCards draw;
    draw.set_number(1);
    ServerInfo_Card *cardInfo = draw.add_cards();
    cardInfo->set_id(7);
    cardInfo->set_name("Cat");

    sendGameEvent(harness, 1, draw);

    EXPECT_TRUE(harness.zone(ZoneNames::DECK)->getCards().isEmpty());
    ASSERT_EQ(harness.zone(ZoneNames::HAND)->getCards().size(), 1);
    CardState *drawn = harness.zone(ZoneNames::HAND)->getCards().first();
    EXPECT_EQ(drawn->getId(), 7);
    EXPECT_EQ(drawn->getName(), QStringLiteral("Cat"));
}

TEST(GameEventHandlerTest, FlipCardHidesAndRevealsTheCard)
{
    GameHarness harness;
    CardState *cat = harness.addCardToZone(ZoneNames::TABLE, QStringLiteral("Cat"), 7);
    ASSERT_NE(cat, nullptr);

    Event_FlipCard flip;
    flip.set_zone_name(ZoneNames::TABLE);
    flip.set_card_id(7);
    flip.set_face_down(true);

    sendGameEvent(harness, 1, flip);

    EXPECT_TRUE(cat->getFaceDown());
    // Flipping face down does not touch the card reference: the client must not
    // learn (or overwrite) the name of a card it is not allowed to see.
    EXPECT_EQ(cat->getName(), QStringLiteral("Cat"));

    // Turning it face up carries the real printing, so the state picks it up.
    flip.set_face_down(false);
    flip.set_card_name("Grizzly Bears");

    sendGameEvent(harness, 1, flip);

    EXPECT_FALSE(cat->getFaceDown());
    EXPECT_EQ(cat->getName(), QStringLiteral("Grizzly Bears"));
}

TEST(GameEventHandlerTest, DestroyCardRemovesItAndDetachesAttachedCards)
{
    GameHarness harness;
    CardState *host = harness.addCardToZone(ZoneNames::TABLE, QStringLiteral("Cat"), 7);
    CardState *guest = harness.addCardToZone(ZoneNames::TABLE, QStringLiteral("Grizzly Bears"), 9);
    ASSERT_NE(host, nullptr);
    ASSERT_NE(guest, nullptr);

    guest->setAttachedTo(host);
    ASSERT_EQ(guest->getAttachedTo(), host);
    ASSERT_EQ(host->getAttachedCards().size(), 1);

    Event_DestroyCard destroy;
    destroy.set_zone_name(ZoneNames::TABLE);
    destroy.set_card_id(7);

    sendGameEvent(harness, 1, destroy);

    CardZoneLogic *table = harness.zone(ZoneNames::TABLE);
    ASSERT_EQ(table->getCards().size(), 1);
    EXPECT_EQ(table->getCards().first(), guest);
    EXPECT_FALSE(table->getCards().contains(host));

    // The attached card survives the host and is left un-attached, ready to be
    // laid out again by the zone.
    EXPECT_EQ(guest->getAttachedTo(), nullptr);
    EXPECT_TRUE(host->getAttachedCards().isEmpty());
    EXPECT_EQ(guest->getZone(), table);
}

TEST(GameEventHandlerTest, CreateTokenUsesDatabasePTUnlessFaceDown)
{
    GameHarness harness;
    CardZoneLogic *table = harness.zone(ZoneNames::TABLE);
    ASSERT_NE(table, nullptr);

    QList<ServerInfo_Card> created;
    QObject::connect(table, &CardZoneLogic::createCardRequested, table,
                     [&created](const ServerInfo_Card &cardInfo, bool) { created.append(cardInfo); });

    Event_CreateToken token;
    token.set_zone_name(ZoneNames::TABLE);
    token.set_card_id(11);
    token.set_card_name("Kitten");
    token.set_x(2);
    token.set_y(1);
    token.set_color("green");
    token.set_annotation("a kitten");
    token.set_destroy_on_zone_change(true);

    sendGameEvent(harness, 1, token);

    ASSERT_EQ(created.size(), 1);
    // The event carries no P/T, so the token takes the printed one.
    EXPECT_EQ(created.first().pt(), "1/1");
    EXPECT_EQ(created.first().name(), "Kitten");
    EXPECT_EQ(created.first().x(), 2);
    EXPECT_EQ(created.first().y(), 1);
    EXPECT_EQ(created.first().color(), "green");
    EXPECT_EQ(created.first().annotation(), "a kitten");
    EXPECT_TRUE(created.first().destroy_on_zone_change());
    EXPECT_FALSE(created.first().face_down());

    // An explicit P/T always wins over the database.
    Event_CreateToken explicitPt = token;
    explicitPt.set_card_id(12);
    explicitPt.set_pt("3/3");

    sendGameEvent(harness, 1, explicitPt);

    ASSERT_EQ(created.size(), 2);
    EXPECT_EQ(created.last().pt(), "3/3");

    // A face-down token must not leak the printed P/T.
    Event_CreateToken faceDown = token;
    faceDown.set_card_id(13);
    faceDown.set_face_down(true);

    sendGameEvent(harness, 1, faceDown);

    ASSERT_EQ(created.size(), 3);
    EXPECT_TRUE(created.last().face_down());
    EXPECT_TRUE(created.last().pt().empty());
}

TEST(GameEventHandlerTest, ConcedeAndUnconcedeUpdatePlayerState)
{
    GameHarness harness;
    ASSERT_NE(harness.addCardToZone(ZoneNames::TABLE, QStringLiteral("Cat"), 7), nullptr);
    EXPECT_FALSE(harness.player()->getConceded());

    Event_PlayerPropertiesChanged properties;
    properties.mutable_player_properties()->set_player_id(1);
    properties.mutable_player_properties()->set_conceded(true);

    GameEventContext concedeContext;
    concedeContext.MutableExtension(Context_Concede::ext);

    sendGameEvent(harness, 1, properties, &concedeContext);

    EXPECT_TRUE(harness.player()->getConceded());
    // Conceding tears the board down for that player.
    EXPECT_TRUE(harness.zone(ZoneNames::TABLE)->getCards().isEmpty());

    Event_PlayerPropertiesChanged back = properties;
    back.mutable_player_properties()->set_conceded(false);

    GameEventContext unconcedeContext;
    unconcedeContext.MutableExtension(Context_Unconcede::ext);

    sendGameEvent(harness, 1, back, &unconcedeContext);

    EXPECT_FALSE(harness.player()->getConceded());
}

TEST(GameEventHandlerTest, SetActivePlayerAndPhaseUpdateGameState)
{
    GameHarness harness;
    GameState *state = harness.game()->getGameState();
    ASSERT_NE(state, nullptr);
    EXPECT_EQ(state->getActivePlayer(), -1);
    EXPECT_EQ(state->getCurrentPhase(), -1);

    Event_SetActivePlayer setPlayer;
    setPlayer.set_active_player_id(1);

    sendGameEvent(harness, 1, setPlayer);

    EXPECT_EQ(state->getActivePlayer(), 1);

    Event_SetActivePhase setPhase;
    setPhase.set_phase(3);

    sendGameEvent(harness, 1, setPhase);

    EXPECT_EQ(state->getCurrentPhase(), 3);
}

TEST(GameEventHandlerTest, CreateSetAndDeletePlayerCounter)
{
    GameHarness harness;

    ServerInfo_Counter counter;
    counter.set_id(1);
    counter.set_name("life");
    counter.set_radius(1);
    counter.set_count(20);

    Event_CreateCounter create;
    create.mutable_counter_info()->CopyFrom(counter);

    sendGameEvent(harness, 1, create);

    CounterState *state = harness.player()->getCounters().value(1);
    ASSERT_NE(state, nullptr);
    EXPECT_EQ(state->getName(), QStringLiteral("life"));
    EXPECT_EQ(state->getValue(), 20);
    EXPECT_EQ(harness.player()->getLifeCounter(), state);

    Event_SetCounter set;
    set.set_counter_id(1);
    set.set_value(15);

    sendGameEvent(harness, 1, set);

    EXPECT_EQ(state->getValue(), 15);

    // Updating a counter that does not exist is a no-op, not a crash.
    Event_SetCounter unknown = set;
    unknown.set_counter_id(99);
    sendGameEvent(harness, 1, unknown);

    EXPECT_FALSE(harness.player()->getCounters().contains(99));

    Event_DelCounter remove;
    remove.set_counter_id(1);

    sendGameEvent(harness, 1, remove);

    EXPECT_FALSE(harness.player()->getCounters().contains(1));
    EXPECT_EQ(harness.player()->getLifeCounter(), nullptr);
}

TEST(GameEventHandlerTest, LeaveRemovesThePlayer)
{
    GameHarness harness;
    ASSERT_NE(harness.addCardToZone(ZoneNames::TABLE, QStringLiteral("Cat"), 7), nullptr);
    ASSERT_NE(harness.game()->getPlayerManager()->getPlayer(1), nullptr);

    Event_Leave leave;
    leave.set_reason(Event_Leave::USER_LEFT);

    sendGameEvent(harness, 1, leave);

    EXPECT_EQ(harness.game()->getPlayerManager()->getPlayer(1), nullptr);
    EXPECT_TRUE(harness.game()->getPlayerManager()->getPlayers().isEmpty());
}

TEST(GameEventHandlerTest, KickedSignalsGameClosed)
{
    GameHarness harness;

    bool closed = false;
    QObject::connect(harness.game()->getGameEventHandler(), &GameEventHandler::gameClosed,
                     [&closed] { closed = true; });

    sendGameEvent(harness, 1, Event_Kicked());

    EXPECT_TRUE(closed);
}

TEST(GameEventHandlerTest, GameClosedMarksTheSessionClosed)
{
    GameHarness harness;
    ASSERT_FALSE(harness.game()->getGameState()->isGameClosed());

    Event_GameStateChanged started;
    started.set_game_started(true);
    sendGameEvent(harness, 1, started);
    ASSERT_TRUE(harness.game()->getGameMetaInfo()->started());

    sendGameEvent(harness, 1, Event_GameClosed());

    EXPECT_TRUE(harness.game()->getGameState()->isGameClosed());
    EXPECT_FALSE(harness.game()->getGameMetaInfo()->started());
}

TEST(GameEventHandlerTest, GameHostChangedMovesTheHostRole)
{
    GameHarness harness;

    ServerInfo_User bob;
    bob.set_name("Bob");
    bob.set_user_level(ServerInfo_User::IsUser);
    ASSERT_NE(harness.game()->getPlayerManager()->addPlayer(2, bob), nullptr);

    sendGameEvent(harness, 2, Event_GameHostChanged());

    EXPECT_EQ(harness.game()->getGameState()->getHostId(), 2);
}
