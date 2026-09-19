#include <gtest/gtest.h>
#include <libcockatrice/network/client/abstract/latency_tracker.h>

TEST(LatencyTrackerTest, EmptyTrackerYieldsZeroedStats)
{
    LatencyTracker tracker;

    const auto stats = tracker.stats();

    EXPECT_EQ(0, stats.sampleCount);
    EXPECT_EQ(0, stats.lastMs);
    EXPECT_EQ(0, stats.medianMs);
    EXPECT_EQ(0, stats.p95Ms);
    EXPECT_EQ(0, stats.maxMs);
}

TEST(LatencyTrackerTest, SingleSampleIsEveryStatistic)
{
    LatencyTracker tracker;
    tracker.addSample(42);

    const auto stats = tracker.stats();

    EXPECT_EQ(1, stats.sampleCount);
    EXPECT_EQ(42, stats.lastMs);
    EXPECT_EQ(42, stats.medianMs);
    EXPECT_EQ(42, stats.p95Ms);
    EXPECT_EQ(42, stats.maxMs);
}

TEST(LatencyTrackerTest, OddSizedWindowMedianAndP95)
{
    LatencyTracker tracker;
    for (qint64 ms : {qint64(50), qint64(10), qint64(30), qint64(20), qint64(40)}) {
        tracker.addSample(ms);
    }

    const auto stats = tracker.stats();

    EXPECT_EQ(5, stats.sampleCount);
    EXPECT_EQ(40, stats.lastMs);
    EXPECT_EQ(30, stats.medianMs);
    // nearest-rank p95 of 5 samples: ceil(4.75) = 5th smallest
    EXPECT_EQ(50, stats.p95Ms);
    EXPECT_EQ(50, stats.maxMs);
}

TEST(LatencyTrackerTest, EvenSizedWindowMedianIsAverageOfMiddleTwo)
{
    LatencyTracker tracker;
    for (qint64 ms : {qint64(10), qint64(20), qint64(30), qint64(40)}) {
        tracker.addSample(ms);
    }

    const auto stats = tracker.stats();

    EXPECT_EQ(4, stats.sampleCount);
    EXPECT_EQ(25, stats.medianMs);
    // nearest-rank p95 of 4 samples: ceil(3.8) = 4th smallest
    EXPECT_EQ(40, stats.p95Ms);
}

TEST(LatencyTrackerTest, WindowEvictsOldestSamples)
{
    LatencyTracker tracker;
    for (int i = 0; i <= 99; ++i) {
        tracker.addSample(i);
    }

    const auto stats = tracker.stats();

    EXPECT_EQ(LatencyTracker::WindowSize, stats.sampleCount);
    EXPECT_EQ(99, stats.lastMs);
    EXPECT_EQ(99, stats.maxMs);
    // window now contains 36..99 (64 samples)
    EXPECT_EQ(67, stats.medianMs); // (67 + 68) / 2 with integer division
    EXPECT_EQ(96, stats.p95Ms);    // ceil(0.95 * 64) - 1 = index 60 -> 36 + 60
}

TEST(LatencyTrackerTest, ClearResetsAllState)
{
    LatencyTracker tracker;
    for (int i = 0; i <= 99; ++i) {
        tracker.addSample(i);
    }

    tracker.clear();
    const auto cleared = tracker.stats();
    EXPECT_EQ(0, cleared.sampleCount);

    tracker.addSample(7);
    const auto stats = tracker.stats();
    EXPECT_EQ(1, stats.sampleCount);
    EXPECT_EQ(7, stats.lastMs);
    EXPECT_EQ(7, stats.medianMs);
}

TEST(LatencyTrackerTest, LastSampleSurvivesWraparound)
{
    LatencyTracker tracker;
    for (int i = 0; i < LatencyTracker::WindowSize; ++i) {
        tracker.addSample(i);
    }
    tracker.addSample(1000);

    EXPECT_EQ(1000, tracker.stats().lastMs);
}

TEST(LatencyTrackerTest, RecentSamplesAreChronologicalOldestFirst)
{
    LatencyTracker tracker;
    for (qint64 ms : {qint64(50), qint64(10), qint64(30)}) {
        tracker.addSample(ms);
    }

    const QList<qint64> samples = tracker.recentSamples();

    EXPECT_EQ((QList<qint64>{50, 10, 30}), samples);
}

TEST(LatencyTrackerTest, RecentSamplesFollowRingBufferWraparound)
{
    LatencyTracker tracker;
    for (int i = 0; i <= 99; ++i) {
        tracker.addSample(i);
    }

    const QList<qint64> samples = tracker.recentSamples();

    ASSERT_EQ(LatencyTracker::WindowSize, samples.size());
    EXPECT_EQ(36, samples.first());
    EXPECT_EQ(99, samples.last());
}

TEST(LatencyTrackerTest, RecentSamplesEmptyOnFreshAndClearedTracker)
{
    LatencyTracker tracker;
    EXPECT_TRUE(tracker.recentSamples().isEmpty());

    tracker.addSample(5);
    tracker.clear();
    EXPECT_TRUE(tracker.recentSamples().isEmpty());
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
