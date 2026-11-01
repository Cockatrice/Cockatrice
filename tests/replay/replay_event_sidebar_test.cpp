#include "client/settings/cache_settings.h"
#include "interface/widgets/replay/replay_event_sidebar.h"
#include "interface/widgets/replay/replay_manager.h"

#include <QLabel>
#include <QListWidget>
#include <QTest>
#include <libcockatrice/protocol/pb/event_game_say.pb.h>
#include <libcockatrice/protocol/pb/event_join.pb.h>
#include <libcockatrice/protocol/pb/game_replay.pb.h>
#include <libcockatrice/settings/interface_settings.h>
#include <vector>

class ReplaySidebarTest : public QObject
{
    Q_OBJECT
private slots:
    void processedPositionAndClick();
    void empty();
    void pausedBufferedRewind();
};

void ReplaySidebarTest::processedPositionAndClick()
{
    auto *replay = new GameReplay;
    for (int i = 0; i < 1001; ++i) {
        auto *container = replay->add_event_list();
        container->set_seconds_elapsed(1);
        container->set_game_id(i);
        if (i < 2) {
            auto *event = container->add_event_list();
            event->set_player_id(i);
            auto *properties = event->MutableExtension(Event_Join::ext)->mutable_player_properties();
            properties->set_player_id(i);
            properties->mutable_user_info()->set_name(i == 0 ? "Alice" : "Bob");
        }
    }
    ReplayManager manager(nullptr, replay);
    ReplayEventSidebar sidebar(nullptr, replay, manager.getReplayTimeline());
    std::vector<int> board;
    QObject::connect(
        &manager, &ReplayManager::eventReplayed, &manager,
        [&board](const GameEventContainer &event, EventProcessingOptions) { board.push_back(event.game_id()); });
    QObject::connect(&manager, &ReplayManager::rewound, &manager, [&board] { board.clear(); });
    QObject::connect(&manager, &ReplayManager::processedEventChanged, &sidebar, &ReplayEventSidebar::setProcessedEvent);
    QObject::connect(&sidebar, &ReplayEventSidebar::eventSelected, &manager, &ReplayManager::setTimeToEvent);

    auto *list = sidebar.findChild<QListWidget *>("replayEventList");
    QVERIFY(list);
    QCOMPARE(list->count(), 2);
    sidebar.resize(400, 300);
    sidebar.show();
    QTest::qWait(50);
    constexpr int currentRole = Qt::UserRole + 7;
    manager.setTime(1000);
    QVERIFY(board.empty());
    QVERIFY(!list->item(0)->data(currentRole).toBool());
    QVERIFY(!list->item(1)->data(currentRole).toBool());

    QTest::mouseClick(list->viewport(), Qt::LeftButton, Qt::NoModifier, list->visualItemRect(list->item(1)).center());
    QCOMPARE(board.size(), size_t(2));
    QCOMPARE(board.back(), 1);
    QVERIFY(!list->item(0)->data(currentRole).toBool());
    QVERIFY(list->item(1)->data(currentRole).toBool());
    QCOMPARE(list->selectedItems().size(), 1);
    QCOMPARE(list->selectedItems().first(), list->item(1));

    QTest::mouseClick(list->viewport(), Qt::LeftButton, Qt::ControlModifier,
                      list->visualItemRect(list->item(0)).center());
    QCOMPARE(board.size(), size_t(1));
    QCOMPARE(board.back(), 0);
    QVERIFY(list->item(0)->data(currentRole).toBool());
    QVERIFY(!list->item(1)->data(currentRole).toBool());
    QCOMPARE(list->selectedItems().size(), 1);
    QCOMPARE(list->selectedItems().first(), list->item(0));
    QTest::mousePress(list->viewport(), Qt::LeftButton, Qt::NoModifier, list->visualItemRect(list->item(0)).center());
    QTest::mouseMove(list->viewport(), list->visualItemRect(list->item(1)).center());
    QTest::mouseRelease(list->viewport(), Qt::LeftButton, Qt::NoModifier, list->visualItemRect(list->item(1)).center());
    QCOMPARE(board.back(), 0);
    QCOMPARE(list->selectedItems().size(), 1);
    QCOMPARE(list->selectedItems().first(), list->item(0));
    list->setCurrentRow(1);
    QVERIFY(list->item(0)->data(currentRole).toBool());
    QCOMPARE(list->selectedItems().size(), 1);
    QCOMPARE(list->selectedItems().first(), list->item(0));
    manager.setTimeToEvent(1);
    QVERIFY(list->item(1)->data(currentRole).toBool());
    manager.setTimeToEvent(999); // No displayed entries after the first two.
    QVERIFY(list->item(1)->data(currentRole).toBool());
    manager.setTime(0);
    QVERIFY(board.empty());
    QVERIFY(!list->item(0)->data(currentRole).toBool());
    QVERIFY(!list->item(1)->data(currentRole).toBool());
    QVERIFY(list->selectedItems().isEmpty());
}

void ReplaySidebarTest::empty()
{
    GameReplay replay;
    ReplayEventSidebar sidebar(nullptr, &replay, {});
    sidebar.show();
    QTest::qWait(10);
    auto *list = sidebar.findChild<QListWidget *>("replayEventList");
    QVERIFY(list);
    QCOMPARE(list->count(), 0);
    QVERIFY(!list->isVisible());
    auto *label = sidebar.findChild<QLabel *>();
    QVERIFY(label);
    QVERIFY(label->isVisible());
    sidebar.setProcessedEvent(-1);
}

void ReplaySidebarTest::pausedBufferedRewind()
{
    auto *replay = new GameReplay;
    for (int i = 0; i < 3; ++i) {
        auto *container = replay->add_event_list();
        container->set_seconds_elapsed(i == 0 ? 0 : 1);
        auto *event = container->add_event_list();
        event->set_player_id(0);
        event->MutableExtension(Event_GameSay::ext)->set_message("Replay position");
    }
    ReplayManager manager(nullptr, replay);
    ReplayEventSidebar sidebar(nullptr, replay, manager.getReplayTimeline());
    QObject::connect(&manager, &ReplayManager::processedEventChanged, &sidebar, &ReplayEventSidebar::setProcessedEvent);
    auto *list = sidebar.findChild<QListWidget *>("replayEventList");
    QVERIFY(list);
    manager.setTimeToEvent(2);
    QCOMPARE(list->currentRow(), 2);
    manager.skipByAmount(-ReplayManager::SMALL_SKIP_MS);
    QCOMPARE(list->currentRow(), 2);
    QTest::qWait(SettingsCache::instance().userInterface().getRewindBufferingMs() + 50);
    QCOMPARE(list->currentRow(), 0);
    QCOMPARE(list->selectedItems().size(), 1);
    QCOMPARE(list->selectedItems().first(), list->item(0));
}

QTEST_MAIN(ReplaySidebarTest)
#include "replay_event_sidebar_test.moc"
