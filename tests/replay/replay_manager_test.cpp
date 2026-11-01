#include "interface/widgets/replay/replay_manager.h"

#include <QTest>
#include <libcockatrice/protocol/pb/game_replay.pb.h>
#include <vector>

namespace
{
GameReplay *makeReplay()
{
    auto *replay = new GameReplay;
    for (int i = 0; i < 1001; ++i) {
        auto *event = replay->add_event_list();
        event->set_seconds_elapsed(1);
        event->set_game_id(i);
    }
    for (int i = 1001; i < 1004; ++i) {
        auto *event = replay->add_event_list();
        event->set_seconds_elapsed(3 + i - 1001);
        event->set_game_id(i);
    }
    return replay;
}

struct ObservedReplay
{
    ReplayManager manager{nullptr, makeReplay()};
    std::vector<int> board;
    int rewinds = 0;
    int time = -1;

    ObservedReplay()
    {
        QObject::connect(
            &manager, &ReplayManager::eventReplayed, &manager,
            [this](const GameEventContainer &event, EventProcessingOptions) { board.push_back(event.game_id()); });
        QObject::connect(&manager, &ReplayManager::rewound, &manager, [this] {
            board.clear();
            ++rewinds;
        });
        QObject::connect(&manager, &ReplayManager::timeChanged, &manager, [this](int t) { time = t; });
    }
};
} // namespace

class ReplayManagerTest : public QObject
{
    Q_OBJECT
private slots:
    void exactEventIdentity();
    void laterSeekCancelsBufferedRewind();
    void playbackResolvesBufferedRewind();
};

void ReplayManagerTest::exactEventIdentity()
{
    ObservedReplay replay;
    const auto &timeline = replay.manager.getReplayTimeline();
    QCOMPARE(timeline.at(0), 1000);
    QCOMPARE(timeline.at(1), 1000);
    QCOMPARE(timeline.at(2), 1001);
    replay.manager.setTimeToEvent(1);
    QCOMPARE(replay.board.size(), size_t(2));
    QCOMPARE(replay.board.back(), 1);
    replay.manager.setTimeToEvent(0);
    QCOMPARE(replay.rewinds, 1);
    QCOMPARE(replay.board.size(), size_t(1));
    QCOMPARE(replay.board.back(), 0);
}

void ReplayManagerTest::laterSeekCancelsBufferedRewind()
{
    ObservedReplay replay;
    replay.manager.setTimeToEvent(1002);
    replay.manager.skipByAmount(-1000);
    QCOMPARE(replay.board.back(), 1002);
    replay.manager.setTimeToEvent(1003);
    const int rewinds = replay.rewinds;
    QTest::qWait(350);
    QCOMPARE(replay.rewinds, rewinds);
    QCOMPARE(replay.board.size(), size_t(1004));
    QCOMPARE(replay.board.back(), 1003);
}

void ReplayManagerTest::playbackResolvesBufferedRewind()
{
    ObservedReplay replay;
    replay.manager.setTimeToEvent(1002);
    replay.manager.skipByAmount(-1000);
    QVERIFY(QMetaObject::invokeMethod(&replay.manager, "replayTimerTimeout", Qt::DirectConnection));
    QCOMPARE(replay.rewinds, 1);
    QCOMPARE(replay.time, 3200);
    QCOMPARE(replay.board.size(), size_t(1002));
    QCOMPARE(replay.board.back(), 1001);
}

QTEST_MAIN(ReplayManagerTest)
#include "replay_manager_test.moc"
