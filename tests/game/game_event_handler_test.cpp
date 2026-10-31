#include "game_test_fixture.h"

#include "gtest/gtest.h"
#include <QPoint>
#include <QString>
#include <libcockatrice/game/board/card_state.h>
#include <libcockatrice/game/game_event_handler.h>
#include <libcockatrice/game/zones/table_zone_logic.h>
#include <libcockatrice/protocol/pb/event_attach_card.pb.h>
#include <libcockatrice/protocol/pb/event_change_zone_properties.pb.h>
#include <libcockatrice/protocol/pb/event_draw_cards.pb.h>
#include <libcockatrice/protocol/pb/event_move_card.pb.h>
#include <libcockatrice/protocol/pb/event_set_card_attr.pb.h>
#include <libcockatrice/protocol/pb/event_set_card_counter.pb.h>
#include <libcockatrice/protocol/pb/game_event_container.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_card.pb.h>
#include <libcockatrice/utility/zone_names.h>
#include <string>

namespace
{

/**
 * Wraps one player-scoped event in the container the server would send and
 * pushes it through the real dispatcher, so the test exercises the same
 * GameEventHandler -> PlayerEventHandler path as a live client.
 */
template <typename EventType> void sendGameEvent(GameHarness &harness, int playerId, const EventType &message)
{
    GameEventContainer container;
    container.set_game_id(42);

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
