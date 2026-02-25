/**
 * @file command_zone_state_test.cpp
 * @brief Unit tests for CommandZoneState pure state machine
 *
 * Tests the extracted visibility/height state logic:
 * - Primary zone: always expanded, cannot be collapsed
 * - Partner/Companion/Background zones: start collapsed, can be toggled
 * - All zones: can be minimized to 25% height
 * - Toggle debouncing: rejects rapid toggles within 200ms
 * - StateChangeResult: flags correctly indicate required side effects
 *
 * @see CommandZoneState for the class under test
 */

#include "cockatrice/src/game/zones/command_zone_state.h"
#include "command_zone_test_common.h"

#include <gtest/gtest.h>

// ============ Parameterized Tests for Non-Primary Zones ============

class NonPrimaryZoneStateTest : public ::testing::TestWithParam<CommandZoneType>
{
protected:
    CommandZoneState *zone = nullptr;

    void TearDown() override
    {
        delete zone;
    }

    void createZone(qreal height = 100.0)
    {
        delete zone;
        zone = new CommandZoneState(height, GetParam());
    }
};

INSTANTIATE_TEST_SUITE_P(NonPrimaryZones,
                         NonPrimaryZoneStateTest,
                         ::testing::Values(CommandZoneType::Partner,
                                           CommandZoneType::Companion,
                                           CommandZoneType::Background),
                         [](const ::testing::TestParamInfo<CommandZoneType> &info) {
                             switch (info.param) {
                                 case CommandZoneType::Partner:
                                     return "Partner";
                                 case CommandZoneType::Companion:
                                     return "Companion";
                                 case CommandZoneType::Background:
                                     return "Background";
                                 default:
                                     return "Unknown";
                             }
                         });

TEST_P(NonPrimaryZoneStateTest, StartsCollapsed)
{
    createZone();
    EXPECT_FALSE(zone->isExpanded()) << "Non-primary zone should start collapsed";
    EXPECT_FALSE(zone->isPrimary());
    EXPECT_EQ(GetParam(), zone->getZoneType());
}

TEST_P(NonPrimaryZoneStateTest, TryToggleExpanded)
{
    createZone();
    ASSERT_FALSE(zone->isExpanded());

    auto result = zone->tryToggleExpanded();

    EXPECT_TRUE(zone->isExpanded()) << "Should expand after toggle";
    EXPECT_TRUE(result.geometryChanged);
}

TEST_P(NonPrimaryZoneStateTest, CurrentHeight_Expanded)
{
    createZone(100.0);
    zone->trySetExpanded(true);
    EXPECT_DOUBLE_EQ(100.0, zone->currentHeight()) << "Expanded zone should have full height";
}

class CommandZoneStateTest : public ::testing::Test
{
protected:
    CommandZoneState *zone;

    void SetUp() override
    {
        zone = nullptr;
    }

    void TearDown() override
    {
        delete zone;
    }

    void createPrimaryZone(qreal height = 100.0)
    {
        delete zone;
        zone = new CommandZoneState(height, CommandZoneType::Primary);
    }

    void createPartnerZone(qreal height = 100.0)
    {
        delete zone;
        zone = new CommandZoneState(height, CommandZoneType::Partner);
    }

    void createCompanionZone(qreal height = 100.0)
    {
        delete zone;
        zone = new CommandZoneState(height, CommandZoneType::Companion);
    }

    void createBackgroundZone(qreal height = 100.0)
    {
        delete zone;
        zone = new CommandZoneState(height, CommandZoneType::Background);
    }
};

// ============ Primary Zone Tests ============

TEST_F(CommandZoneStateTest, PrimaryZone_AlwaysExpanded)
{
    createPrimaryZone();

    EXPECT_TRUE(zone->isExpanded()) << "Primary zone should start expanded";
    EXPECT_TRUE(zone->isPrimary()) << "Primary zone should report isPrimary() true";
}

TEST_F(CommandZoneStateTest, PrimaryZone_TryToggleExpanded_NoOp)
{
    createPrimaryZone();
    ASSERT_TRUE(zone->isExpanded());

    auto result = zone->tryToggleExpanded();

    EXPECT_TRUE(zone->isExpanded()) << "Primary zone should remain expanded after tryToggleExpanded";
    EXPECT_FALSE(result.geometryChanged) << "No geometry change for Primary zone toggle";
    EXPECT_FALSE(result.shouldEmitExpanded) << "No expanded signal for Primary zone toggle";
}

TEST_F(CommandZoneStateTest, CurrentHeight_PrimaryFull)
{
    createPrimaryZone(100.0);

    EXPECT_DOUBLE_EQ(100.0, zone->currentHeight()) << "Primary zone should have full height";
}

TEST_F(CommandZoneStateTest, CurrentHeight_PrimaryMinimized)
{
    createPrimaryZone(100.0);

    auto result = zone->tryToggleMinimized();

    EXPECT_DOUBLE_EQ(25.0, zone->currentHeight()) << "Minimized primary zone should be 25% height";
    EXPECT_TRUE(result.geometryChanged);
    EXPECT_TRUE(result.shouldEmitMinimized);
}

// ============ Partner Zone Tests (Partner-specific behaviors) ============

TEST_F(CommandZoneStateTest, PartnerZone_TryToggleExpanded_TwiceCollapses)
{
    createPartnerZone();
    zone->tryToggleExpanded();
    ASSERT_TRUE(zone->isExpanded());

    zone->resetDebounce();
    auto result = zone->tryToggleExpanded();

    EXPECT_FALSE(zone->isExpanded()) << "Partner zone should collapse after second toggle";
    EXPECT_TRUE(result.geometryChanged);
    EXPECT_TRUE(result.shouldEmitExpanded);
}

TEST_F(CommandZoneStateTest, CurrentHeight_PartnerCollapsed)
{
    createPartnerZone(100.0);
    ASSERT_FALSE(zone->isExpanded());

    EXPECT_DOUBLE_EQ(0.0, zone->currentHeight()) << "Collapsed partner zone should have 0 height";
}

TEST_F(CommandZoneStateTest, CurrentHeight_PartnerExpandedMinimized)
{
    createPartnerZone(100.0);
    zone->trySetExpanded(true);

    zone->tryToggleMinimized();

    EXPECT_DOUBLE_EQ(25.0, zone->currentHeight()) << "Expanded+minimized partner zone should be 25% height";
}

// ============ Debounce Tests ============

TEST_F(CommandZoneStateTest, TryToggleDebounce_RejectsRapidCalls)
{
    createPartnerZone();
    ASSERT_FALSE(zone->isExpanded());

    zone->tryToggleExpanded();
    ASSERT_TRUE(zone->isExpanded());

    auto result = zone->tryToggleExpanded();

    EXPECT_TRUE(zone->isExpanded()) << "Rapid toggle should be rejected (debounced)";
    EXPECT_FALSE(result.geometryChanged) << "Debounced toggle should not change geometry";
}

TEST_F(CommandZoneStateTest, TryToggleDebounce_AcceptsAfterReset)
{
    createPartnerZone();

    zone->tryToggleExpanded();
    ASSERT_TRUE(zone->isExpanded());

    zone->resetDebounce();

    auto result = zone->tryToggleExpanded();

    EXPECT_FALSE(zone->isExpanded()) << "Toggle after reset should work";
    EXPECT_TRUE(result.geometryChanged);
}

// ============ Minimize Tests ============

TEST_F(CommandZoneStateTest, Zone_StartsNotMinimized)
{
    createPrimaryZone();

    EXPECT_FALSE(zone->isMinimized()) << "Zone should start not minimized";
}

TEST_F(CommandZoneStateTest, TryToggleMinimized_TogglesState)
{
    createPrimaryZone();
    ASSERT_FALSE(zone->isMinimized());

    auto result = zone->tryToggleMinimized();

    EXPECT_TRUE(zone->isMinimized()) << "Zone should be minimized after toggle";
    EXPECT_TRUE(result.geometryChanged);
    EXPECT_TRUE(result.shouldEmitMinimized);
}

TEST_F(CommandZoneStateTest, TryToggleMinimized_Twice_RestoresState)
{
    createPrimaryZone();
    zone->tryToggleMinimized();
    ASSERT_TRUE(zone->isMinimized());

    auto result = zone->tryToggleMinimized();

    EXPECT_FALSE(zone->isMinimized()) << "Zone should be restored after second toggle";
    EXPECT_TRUE(result.geometryChanged);
    EXPECT_TRUE(result.shouldEmitMinimized);
}

TEST_F(CommandZoneStateTest, TryToggleMinimized_NoOpWhenCollapsed)
{
    createPartnerZone();
    ASSERT_TRUE(zone->isCollapsed());

    auto result = zone->tryToggleMinimized();

    EXPECT_FALSE(result.geometryChanged) << "Collapsed zone should not toggle minimized";
    EXPECT_FALSE(result.shouldEmitMinimized);
}

// ============ StateChangeResult Tests ============

TEST_F(CommandZoneStateTest, TrySetExpanded_ReturnsCorrectResult_OnExpand)
{
    createPartnerZone();
    ASSERT_FALSE(zone->isExpanded());

    auto result = zone->trySetExpanded(true);

    EXPECT_TRUE(result.geometryChanged);
    EXPECT_TRUE(result.shouldEmitExpanded);
    EXPECT_FALSE(result.shouldEmitMinimized);
}

TEST_F(CommandZoneStateTest, TrySetExpanded_ReturnsCorrectResult_OnCollapse)
{
    createPartnerZone();
    zone->trySetExpanded(true);
    ASSERT_TRUE(zone->isExpanded());

    auto result = zone->trySetExpanded(false);

    EXPECT_TRUE(result.geometryChanged);
    EXPECT_TRUE(result.shouldEmitExpanded);
    EXPECT_FALSE(result.shouldEmitMinimized) << "Not minimized before collapse";
}

TEST_F(CommandZoneStateTest, TrySetExpanded_ReturnsCorrectResult_OnCollapseWhileMinimized)
{
    createPartnerZone();
    zone->trySetExpanded(true);
    zone->tryToggleMinimized();
    ASSERT_TRUE(zone->isMinimized());

    auto result = zone->trySetExpanded(false);

    EXPECT_TRUE(result.geometryChanged);
    EXPECT_TRUE(result.shouldEmitExpanded);
    EXPECT_TRUE(result.shouldEmitMinimized) << "Was minimized before collapse";
}

TEST_F(CommandZoneStateTest, TrySetExpanded_NoChange_ReturnsNoChangeResult)
{
    createPartnerZone();
    zone->trySetExpanded(true);
    ASSERT_TRUE(zone->isExpanded());

    auto result = zone->trySetExpanded(true);

    EXPECT_FALSE(result.geometryChanged) << "No change when already in target state";
    EXPECT_FALSE(result.shouldEmitExpanded);
    EXPECT_FALSE(result.shouldEmitMinimized);
}

TEST_F(CommandZoneStateTest, TrySetExpanded_Blocked_ReturnsNoChangeResult)
{
    createPartnerZone();
    zone->trySetExpanded(true);
    ASSERT_TRUE(zone->isExpanded());

    auto result = zone->trySetExpanded(false, true);

    EXPECT_FALSE(result.geometryChanged) << "Collapse blocked";
    EXPECT_TRUE(zone->isExpanded()) << "Should still be expanded";
}

TEST_F(CommandZoneStateTest, TryToggleMinimized_ReturnsCorrectResult)
{
    createPrimaryZone();

    auto result = zone->tryToggleMinimized();

    EXPECT_TRUE(result.geometryChanged);
    EXPECT_FALSE(result.shouldEmitExpanded) << "Minimized toggle doesn't change expanded state";
    EXPECT_TRUE(result.shouldEmitMinimized);
}

// ============ Edge Cases ============

TEST_F(CommandZoneStateTest, PartnerCollapsedMinimized_ZeroHeight)
{
    createPartnerZone(100.0);
    ASSERT_FALSE(zone->isExpanded());

    zone->tryToggleMinimized();

    EXPECT_DOUBLE_EQ(0.0, zone->currentHeight()) << "Collapsed partner zone should be 0 height even when minimized";
}

TEST_F(CommandZoneStateTest, DifferentZoneHeight)
{
    createPrimaryZone(200.0);

    EXPECT_DOUBLE_EQ(200.0, zone->currentHeight());
    EXPECT_DOUBLE_EQ(200.0, zone->getZoneHeight());

    zone->tryToggleMinimized();

    EXPECT_DOUBLE_EQ(50.0, zone->currentHeight()) << "25% of 200 should be 50";
}

TEST_F(CommandZoneStateTest, NoChange_StaticMethod)
{
    auto result = StateChangeResult::noChange();

    EXPECT_FALSE(result.geometryChanged);
    EXPECT_FALSE(result.shouldEmitExpanded);
    EXPECT_FALSE(result.shouldEmitMinimized);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
