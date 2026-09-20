#include "client/lag_monitor.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QEvent>
#include <QLoggingCategory>
#include <gtest/gtest.h>

namespace
{

/// Timestamps are taken at recording time; allow generous scheduler slack.
constexpr qint64 TIMESTAMP_SLACK_MS = 10000;

} // namespace

class LagMonitorTest : public ::testing::Test
{
protected:
    LagMonitor monitor;
};

TEST_F(LagMonitorTest, GapAtOrBelowThresholdIsIgnored)
{
    monitor.recordGap(0);
    monitor.recordGap(LagMonitor::TICK_INTERVAL_MS);
    monitor.recordGap(LagMonitor::STALL_THRESHOLD_MS);

    EXPECT_TRUE(monitor.recentStalls().isEmpty());
}

TEST_F(LagMonitorTest, GapAboveThresholdIsRecorded)
{
    monitor.recordGap(LagMonitor::STALL_THRESHOLD_MS + 1);

    const QList<LagMonitor::StallRecord> stalls = monitor.recentStalls();
    ASSERT_EQ(1, stalls.size());
    EXPECT_EQ(LagMonitor::STALL_THRESHOLD_MS + 1, stalls.first().durationMs);
}

TEST_F(LagMonitorTest, RecordedTimestampIsFresh)
{
    monitor.recordGap(LagMonitor::STALL_THRESHOLD_MS + 1);

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    ASSERT_EQ(1, monitor.recentStalls().size());
    EXPECT_LE(qAbs(monitor.recentStalls().first().timestampMsSinceEpoch - now), TIMESTAMP_SLACK_MS);
}

TEST_F(LagMonitorTest, RecordsAreTrimmedToMaxOldestFirst)
{
    for (int i = 0; i < LagMonitor::MAX_RECORDED_STALLS + 5; ++i) {
        monitor.recordGap(LagMonitor::STALL_THRESHOLD_MS + 1 + i);
    }

    const QList<LagMonitor::StallRecord> stalls = monitor.recentStalls();
    ASSERT_EQ(LagMonitor::MAX_RECORDED_STALLS, stalls.size());
    EXPECT_EQ(LagMonitor::STALL_THRESHOLD_MS + 6, stalls.first().durationMs);
    EXPECT_EQ(LagMonitor::STALL_THRESHOLD_MS + 5 + LagMonitor::MAX_RECORDED_STALLS, stalls.last().durationMs);
}

TEST_F(LagMonitorTest, GapAtPlausibilityCapIsKept)
{
    monitor.recordGap(LagMonitor::MAX_PLAUSIBLE_STALL_MS);

    ASSERT_EQ(1, monitor.recentStalls().size());
    EXPECT_EQ(LagMonitor::MAX_PLAUSIBLE_STALL_MS, monitor.recentStalls().first().durationMs);
}

TEST_F(LagMonitorTest, GapBeyondPlausibilityCapIsDropped)
{
    monitor.recordGap(LagMonitor::MAX_PLAUSIBLE_STALL_MS + 1);

    EXPECT_TRUE(monitor.recentStalls().isEmpty());
}

TEST_F(LagMonitorTest, ClearStallsEmptiesList)
{
    monitor.recordGap(LagMonitor::STALL_THRESHOLD_MS + 1);
    ASSERT_EQ(1, monitor.recentStalls().size());

    monitor.clearStalls();

    EXPECT_TRUE(monitor.recentStalls().isEmpty());
}

TEST_F(LagMonitorTest, ApplicationStateChangeDoesNotRecordAStall)
{
    QObject probe;
    QEvent event(QEvent::ApplicationStateChange);

    QCoreApplication::sendEvent(&probe, &event);

    EXPECT_TRUE(monitor.recentStalls().isEmpty());
}

int main(int argc, char **argv)
{
    QLoggingCategory::setFilterRules("lag_monitor.*=false");
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
