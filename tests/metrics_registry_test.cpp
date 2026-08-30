#include <QList>
#include <gtest/gtest.h>
#include <metrics_registry.h>

TEST(MetricsRegistryTest, EmptyRegistryHasZeroedCounters)
{
    MetricsRegistry registry;

    EXPECT_EQ(0, registry.totalCommands());
    EXPECT_EQ(0, registry.totalTimeMs());
    EXPECT_EQ(0, registry.activeTypeCount());
    EXPECT_EQ(0, registry.getGameStartSnapshot().count);
}

TEST(MetricsRegistryTest, SampleIsRecordedInTotalsAndSlot)
{
    MetricsRegistry registry;
    registry.observeCommand(MetricsRegistry::typeIdFor(0, 1000), 7);

    EXPECT_EQ(1, registry.totalCommands());
    EXPECT_EQ(7, registry.totalTimeMs());
    EXPECT_EQ(1, registry.activeTypeCount());

    const auto stats = registry.collectActiveStats();
    ASSERT_EQ(1, stats.size());
    EXPECT_EQ(MetricsRegistry::typeIdFor(0, 1000), stats[0].typeId);
    EXPECT_EQ(1, stats[0].count);
    EXPECT_EQ(7, stats[0].totalMs);
}

TEST(MetricsRegistryTest, KindEncodingSeparatesSameExtensionNumber)
{
    MetricsRegistry registry;
    const int sessionPing = MetricsRegistry::typeIdFor(0, 1000);
    const int roomLeaveRoom = MetricsRegistry::typeIdFor(1, 1000);
    ASSERT_NE(sessionPing, roomLeaveRoom);

    registry.observeCommand(sessionPing, 1);
    registry.observeCommand(roomLeaveRoom, 5000);

    EXPECT_EQ(2, registry.activeTypeCount());
}

TEST(MetricsRegistryTest, OutOfRangeIdsLandInOverflowSlot)
{
    MetricsRegistry registry;
    registry.observeCommand(-1, 4);
    registry.observeCommand(MetricsRegistry::MaxTypes + 12345, 4);

    EXPECT_EQ(2, registry.totalCommands());
    EXPECT_EQ(1, registry.activeTypeCount()); // both collapsed into one slot
    EXPECT_EQ(8, registry.totalTimeMs());
}

TEST(MetricsRegistryTest, NegativeDurationsAreClamped)
{
    MetricsRegistry registry;
    registry.observeCommand(MetricsRegistry::typeIdFor(0, 1000), -50);

    EXPECT_EQ(0, registry.totalTimeMs());
}

TEST(MetricsRegistryTest, GameStartTrackedSeparatelyFromCommands)
{
    MetricsRegistry registry;
    registry.observeGameStartDurationMs(120);

    EXPECT_EQ(0, registry.totalCommands());
    EXPECT_EQ(0, registry.totalTimeMs());
    EXPECT_EQ(0, registry.activeTypeCount());

    const auto snapshot = registry.getGameStartSnapshot();
    EXPECT_EQ(1, snapshot.count);
    EXPECT_EQ(120, snapshot.totalMs);
}