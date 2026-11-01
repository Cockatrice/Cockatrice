#include "interface/widgets/replay/replay_event_index.h"

#include <QTest>
#include <libcockatrice/protocol/pb/context_ready_start.pb.h>
#include <libcockatrice/protocol/pb/event_attach_card.pb.h>
#include <libcockatrice/protocol/pb/event_draw_cards.pb.h>
#include <libcockatrice/protocol/pb/event_flip_card.pb.h>
#include <libcockatrice/protocol/pb/event_game_state_changed.pb.h>
#include <libcockatrice/protocol/pb/event_join.pb.h>
#include <libcockatrice/protocol/pb/event_leave.pb.h>
#include <libcockatrice/protocol/pb/event_move_card.pb.h>
#include <libcockatrice/protocol/pb/event_player_properties_changed.pb.h>
#include <libcockatrice/protocol/pb/event_set_card_attr.pb.h>
#include <libcockatrice/protocol/pb/event_set_counter.pb.h>
#include <libcockatrice/protocol/pb/game_replay.pb.h>

class ReplayIndexTest : public QObject
{
    Q_OBJECT
private slots:
    void actorsAndPresence();
    void repeatedActionsAndCards();
    void attachAndUnattach();
    void snapshotsReplaceOnlyIncludedPlayerState();
    void explicitEmptyMoveNameClearsIdentity();
    void explicitEmptyFlipNameClearsIdentity();
    void faceDownFlipPreservesKnownIdentity();
};

void ReplayIndexTest::actorsAndPresence()
{
    GameReplay replay;
    auto *join = replay.add_event_list()->add_event_list();
    join->set_player_id(7);
    auto *properties = join->MutableExtension(Event_Join::ext)->mutable_player_properties();
    properties->set_spectator(true);
    properties->mutable_user_info()->set_name("Alice");

    auto *change = replay.add_event_list();
    change->mutable_context()->MutableExtension(Context_ReadyStart::ext);
    auto *event = change->add_event_list();
    event->set_player_id(7);
    event->MutableExtension(Event_PlayerPropertiesChanged::ext)->mutable_player_properties()->set_ready_start(true);

    auto *leave = replay.add_event_list()->add_event_list();
    leave->set_player_id(7);
    leave->MutableExtension(Event_Leave::ext);
    for (int id : {0, -1}) {
        auto *extra = replay.add_event_list();
        extra->mutable_context()->MutableExtension(Context_ReadyStart::ext);
        auto *ready = extra->add_event_list();
        ready->set_player_id(7);
        ready->MutableExtension(Event_PlayerPropertiesChanged::ext)->mutable_player_properties()->set_player_id(id);
    }
    const auto entries = buildReplayEventEntries(&replay, {0, 1000, 2000, 3000, 4000});
    QCOMPARE(entries.size(), 5);
    QCOMPARE(entries[0].actor, QStringLiteral("Alice"));
    QVERIFY(entries[0].detail.contains("spectator"));
    QCOMPARE(entries[1].actor, QStringLiteral("Alice"));
    QVERIFY(entries[1].detail.contains("ready"));
    QCOMPARE(entries[2].actor, QStringLiteral("Alice"));
    QVERIFY(entries[2].detail.contains("spectating"));
    QCOMPARE(entries[3].actor, QStringLiteral("Player 0"));
    QCOMPARE(entries[4].actor, QStringLiteral("Alice"));
}

void ReplayIndexTest::repeatedActionsAndCards()
{
    GameReplay replay;
    auto *container = replay.add_event_list();
    auto *event = container->add_event_list();
    event->set_player_id(2);
    auto *draw = event->MutableExtension(Event_DrawCards::ext);
    draw->set_number(2);
    draw->add_cards()->set_name("Island");
    draw->add_cards()->set_name("Island");
    for (int i = 0; i < 2; ++i) {
        auto *tap = container->add_event_list();
        tap->set_player_id(2);
        auto *attribute = tap->MutableExtension(Event_SetCardAttr::ext);
        attribute->set_zone_name("table");
        attribute->set_card_id(4);
        attribute->set_attribute(CardAttribute::AttrTapped);
        attribute->set_attr_value("1");
    }
    const auto entries = buildReplayEventEntries(&replay, {0});
    QCOMPARE(entries.size(), 1);
    QVERIFY(entries[0].detail.contains("Island, Island"));
    QCOMPARE(entries[0].detail.count("tapped"), 2);
    QVERIFY(entries[0].badge.contains("+2"));
}

void ReplayIndexTest::attachAndUnattach()
{
    GameReplay replay;
    for (int i = 0; i < 2; ++i) {
        auto *event = replay.add_event_list()->add_event_list();
        event->set_player_id(2);
        auto *attach = event->MutableExtension(Event_AttachCard::ext);
        attach->set_card_id(9);
        attach->set_start_zone("table");
        if (i == 0) {
            attach->set_target_player_id(0);
            attach->set_target_card_id(1);
            attach->set_target_zone("table");
        }
    }
    const auto entries = buildReplayEventEntries(&replay, {0, 1000});
    QCOMPARE(entries.size(), 2);
    QVERIFY(entries[0].detail.startsWith("attached"));
    QVERIFY(entries[1].detail.startsWith("unattached"));
}

void ReplayIndexTest::snapshotsReplaceOnlyIncludedPlayerState()
{
    GameReplay replay;
    auto *state = replay.add_event_list()->add_event_list()->MutableExtension(Event_GameStateChanged::ext);
    for (int id : {1, 10}) {
        auto *player = state->add_player_list();
        player->mutable_properties()->set_player_id(id);
        auto *zone = player->add_zone_list();
        zone->set_name("table");
        auto *card = zone->add_card_list();
        card->set_id(4);
        card->set_name(id == 1 ? "Old card" : "Other card");
        auto *counter = player->add_counter_list();
        counter->set_id(3);
        counter->set_name(id == 1 ? "Old counter" : "Other counter");
    }
    auto *replacement = replay.add_event_list()->add_event_list()->MutableExtension(Event_GameStateChanged::ext);
    auto *player = replacement->add_player_list();
    player->mutable_properties()->set_player_id(1);
    auto *zone = player->add_zone_list();
    zone->set_name("table");
    zone->add_card_list()->set_id(4); // Same ID now has no recorded name.
    player->add_counter_list()->set_id(3);

    for (int id : {1, 10}) {
        auto *container = replay.add_event_list();
        auto *tapEvent = container->add_event_list();
        tapEvent->set_player_id(id);
        auto *tap = tapEvent->MutableExtension(Event_SetCardAttr::ext);
        tap->set_zone_name("table");
        tap->set_card_id(4);
        tap->set_attribute(CardAttribute::AttrTapped);
        tap->set_attr_value("1");
        auto *counterEvent = container->add_event_list();
        counterEvent->set_player_id(id);
        auto *counter = counterEvent->MutableExtension(Event_SetCounter::ext);
        counter->set_counter_id(3);
        counter->set_value(5);
    }
    const auto entries = buildReplayEventEntries(&replay, {});
    QCOMPARE(entries.size(), 3);
    QCOMPARE(entries[1].eventIndex, 2);
    QVERIFY(entries[1].detail.contains("tapped a card."));
    QVERIFY(entries[1].detail.contains("set a counter to 5."));
    QVERIFY(!entries[1].detail.contains("Old"));
    QVERIFY(entries[2].detail.contains("Other card"));
    QVERIFY(entries[2].detail.contains("Other counter"));
}

void ReplayIndexTest::explicitEmptyMoveNameClearsIdentity()
{
    GameReplay replay;
    auto *drawEvent = replay.add_event_list()->add_event_list();
    drawEvent->set_player_id(1);
    auto *draw = drawEvent->MutableExtension(Event_DrawCards::ext);
    draw->set_number(1);
    auto *card = draw->add_cards();
    card->set_id(4);
    card->set_name("Old identity");
    auto *moveEvent = replay.add_event_list()->add_event_list();
    moveEvent->set_player_id(1);
    auto *move = moveEvent->MutableExtension(Event_MoveCard::ext);
    move->set_start_player_id(1);
    move->set_target_player_id(1);
    move->set_start_zone("hand");
    move->set_target_zone("table");
    move->set_card_id(4);
    move->set_new_card_id(8);
    move->set_card_name(""); // Present and empty resets the native card reference.
    move->set_face_down(true);
    auto *tapEvent = replay.add_event_list()->add_event_list();
    tapEvent->set_player_id(1);
    auto *tap = tapEvent->MutableExtension(Event_SetCardAttr::ext);
    tap->set_zone_name("table");
    tap->set_card_id(8);
    tap->set_attribute(CardAttribute::AttrTapped);
    tap->set_attr_value("1");

    const auto entries = buildReplayEventEntries(&replay, {});
    QVERIFY(entries[1].detail.contains("a face-down card"));
    QVERIFY(!entries[1].detail.contains("Old identity"));
    QCOMPARE(entries[2].detail, QStringLiteral("tapped a card."));

    // An omitted name preserves the known identity, as the game handler does.
    move->clear_card_name();
    const auto retained = buildReplayEventEntries(&replay, {});
    QVERIFY(retained[1].detail.contains("Old identity"));
    QCOMPARE(retained[2].detail, QStringLiteral("tapped Old identity."));
}

void ReplayIndexTest::explicitEmptyFlipNameClearsIdentity()
{
    GameReplay replay;
    for (const char *name : {"Old identity", ""}) {
        auto *event = replay.add_event_list()->add_event_list();
        event->set_player_id(1);
        auto *flip = event->MutableExtension(Event_FlipCard::ext);
        flip->set_zone_name("table");
        flip->set_card_id(4);
        flip->set_card_name(name);
    }
    auto *event = replay.add_event_list()->add_event_list();
    event->set_player_id(1);
    auto *tap = event->MutableExtension(Event_SetCardAttr::ext);
    tap->set_zone_name("table");
    tap->set_card_id(4);
    tap->set_attribute(CardAttribute::AttrTapped);
    tap->set_attr_value("1");
    const auto entries = buildReplayEventEntries(&replay, {});
    QCOMPARE(entries[2].detail, QStringLiteral("tapped a card."));
}

void ReplayIndexTest::faceDownFlipPreservesKnownIdentity()
{
    GameReplay replay;
    for (bool faceDown : {false, true}) {
        auto *event = replay.add_event_list()->add_event_list();
        event->set_player_id(1);
        auto *flip = event->MutableExtension(Event_FlipCard::ext);
        flip->set_zone_name("table");
        flip->set_card_id(4);
        flip->set_face_down(faceDown);
        flip->set_card_name(faceDown ? "" : "Known identity");
    }
    const auto entries = buildReplayEventEntries(&replay, {});
    QCOMPARE(entries[1].detail, QStringLiteral("flipped Known identity face down."));
}

QTEST_GUILESS_MAIN(ReplayIndexTest)
#include "replay_event_index_test.moc"
