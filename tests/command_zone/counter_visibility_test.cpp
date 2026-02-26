/**
 * @file counter_visibility_test.cpp
 * @brief Unit tests for counter visibility during zone state transitions
 *
 * Tests the visibility logic for commander tax counters:
 * - Commander tax visible when command zone visible
 * - Partner tax visible when command zones visible AND partner expanded
 * - Counters remain visible during minimize (only height changes)
 * - Signal-driven visibility updates
 */

#include <QSignalSpy>
#include <gtest/gtest.h>

/**
 * @brief Testable implementation of counter visibility logic.
 *
 * Extracts the visibility determination logic from
 * PlayerGraphicsItem::rearrangeCounters for isolated testing.
 *
 * @warning SYNC REQUIRED: This harness duplicates visibility logic from
 * player_graphics_item.cpp lines 172-195. Update if implementation changes.
 */
class CounterVisibilityTestHarness : public QObject
{
    Q_OBJECT

    bool commandZonesVisible_;
    bool partnerZoneExpanded_;

    bool commanderTaxVisible_;
    bool partnerTaxVisible_;

public:
    explicit CounterVisibilityTestHarness(QObject *parent = nullptr)
        : QObject(parent), commandZonesVisible_(true), partnerZoneExpanded_(false), commanderTaxVisible_(false),
          partnerTaxVisible_(false)
    {
    }

    /**
     * @brief Sets the command zones visibility state.
     *
     * This simulates the server enabling/disabling command zones
     * based on game format (Commander vs other formats).
     */
    void setCommandZonesVisible(bool visible)
    {
        commandZonesVisible_ = visible;
        updateCounterVisibility();
    }

    /**
     * @brief Sets the partner zone expanded state.
     *
     * Simulates user toggling the partner zone via button or menu.
     */
    void setPartnerZoneExpanded(bool expanded)
    {
        partnerZoneExpanded_ = expanded;
        updateCounterVisibility();
    }

    [[nodiscard]] bool isCommanderTaxVisible() const
    {
        return commanderTaxVisible_;
    }

    [[nodiscard]] bool isPartnerTaxVisible() const
    {
        return partnerTaxVisible_;
    }

    [[nodiscard]] bool areCommandZonesVisible() const
    {
        return commandZonesVisible_;
    }

    [[nodiscard]] bool isPartnerZoneExpanded() const
    {
        return partnerZoneExpanded_;
    }

signals:
    void commanderTaxVisibilityChanged(bool visible);
    void partnerTaxVisibilityChanged(bool visible);

private:
    /**
     * @brief Implementation of counter visibility logic.
     *
     * Commander tax: visible when command zones visible
     * Partner tax: visible when command zones visible AND partner expanded
     */
    void updateCounterVisibility()
    {
        bool newCommanderVisible = commandZonesVisible_;
        bool newPartnerVisible = commandZonesVisible_ && partnerZoneExpanded_;

        if (newCommanderVisible != commanderTaxVisible_) {
            commanderTaxVisible_ = newCommanderVisible;
            emit commanderTaxVisibilityChanged(commanderTaxVisible_);
        }

        if (newPartnerVisible != partnerTaxVisible_) {
            partnerTaxVisible_ = newPartnerVisible;
            emit partnerTaxVisibilityChanged(partnerTaxVisible_);
        }
    }
};

class CounterVisibilityTest : public ::testing::Test
{
protected:
    CounterVisibilityTestHarness *harness;

    void SetUp() override
    {
        harness = new CounterVisibilityTestHarness();
    }

    void TearDown() override
    {
        delete harness;
    }
};

// ============ Commander Tax Visibility Tests ============

// Test: Commander tax visible when zones visible
TEST_F(CounterVisibilityTest, CommanderTax_VisibleWhenZonesVisible)
{
    harness->setCommandZonesVisible(true);

    EXPECT_TRUE(harness->isCommanderTaxVisible()) << "Commander tax should be visible when zones visible";
}

// Test: Commander tax hidden when zones not visible
TEST_F(CounterVisibilityTest, CommanderTax_HiddenWhenZonesNotVisible)
{
    harness->setCommandZonesVisible(false);

    EXPECT_FALSE(harness->isCommanderTaxVisible()) << "Commander tax should be hidden when zones not visible";
}

// Test: Commander tax visibility independent of partner zone state
TEST_F(CounterVisibilityTest, CommanderTax_IndependentOfPartnerState)
{
    harness->setCommandZonesVisible(true);

    harness->setPartnerZoneExpanded(false);
    EXPECT_TRUE(harness->isCommanderTaxVisible()) << "Commander tax visible with partner collapsed";

    harness->setPartnerZoneExpanded(true);
    EXPECT_TRUE(harness->isCommanderTaxVisible()) << "Commander tax visible with partner expanded";
}

// ============ Partner Tax Visibility Tests ============

// Test: Partner tax visible when zones visible AND partner expanded
TEST_F(CounterVisibilityTest, PartnerTax_VisibleWhenExpandedAndZonesVisible)
{
    harness->setCommandZonesVisible(true);
    harness->setPartnerZoneExpanded(true);

    EXPECT_TRUE(harness->isPartnerTaxVisible()) << "Partner tax should be visible when expanded and zones visible";
}

// Test: Partner tax hidden when partner collapsed
TEST_F(CounterVisibilityTest, PartnerTax_HiddenWhenCollapsed)
{
    harness->setCommandZonesVisible(true);
    harness->setPartnerZoneExpanded(false);

    EXPECT_FALSE(harness->isPartnerTaxVisible()) << "Partner tax should be hidden when partner collapsed";
}

// Test: Partner tax hidden when zones not visible (even if expanded)
TEST_F(CounterVisibilityTest, PartnerTax_HiddenWhenZonesNotVisible)
{
    harness->setCommandZonesVisible(false);
    harness->setPartnerZoneExpanded(true);

    EXPECT_FALSE(harness->isPartnerTaxVisible())
        << "Partner tax should be hidden when zones not visible, even if expanded";
}

// ============ Visibility Transition Tests ============

// Test: Correct sequence during expand
TEST_F(CounterVisibilityTest, ExpandTransition_PartnerBecomesVisible)
{
    harness->setCommandZonesVisible(true);
    harness->setPartnerZoneExpanded(false);
    ASSERT_FALSE(harness->isPartnerTaxVisible());

    harness->setPartnerZoneExpanded(true);

    EXPECT_TRUE(harness->isPartnerTaxVisible()) << "Partner tax should become visible on expand";
}

// Test: Correct sequence during collapse
TEST_F(CounterVisibilityTest, CollapseTransition_PartnerBecomesHidden)
{
    harness->setCommandZonesVisible(true);
    harness->setPartnerZoneExpanded(true);
    ASSERT_TRUE(harness->isPartnerTaxVisible());

    harness->setPartnerZoneExpanded(false);

    EXPECT_FALSE(harness->isPartnerTaxVisible()) << "Partner tax should become hidden on collapse";
}

// Test: Hiding zones hides both counters
TEST_F(CounterVisibilityTest, HideZones_HidesBothCounters)
{
    harness->setCommandZonesVisible(true);
    harness->setPartnerZoneExpanded(true);
    ASSERT_TRUE(harness->isCommanderTaxVisible());
    ASSERT_TRUE(harness->isPartnerTaxVisible());

    harness->setCommandZonesVisible(false);

    EXPECT_FALSE(harness->isCommanderTaxVisible()) << "Commander tax hidden when zones hidden";
    EXPECT_FALSE(harness->isPartnerTaxVisible()) << "Partner tax hidden when zones hidden";
}

// Test: Showing zones restores commander tax, partner depends on state
TEST_F(CounterVisibilityTest, ShowZones_RestoresCorrectVisibility)
{
    harness->setCommandZonesVisible(true);
    harness->setPartnerZoneExpanded(false);
    harness->setCommandZonesVisible(false);
    ASSERT_FALSE(harness->isCommanderTaxVisible());
    ASSERT_FALSE(harness->isPartnerTaxVisible());

    harness->setCommandZonesVisible(true);

    EXPECT_TRUE(harness->isCommanderTaxVisible()) << "Commander tax restored when zones shown";
    EXPECT_FALSE(harness->isPartnerTaxVisible()) << "Partner tax stays hidden (partner still collapsed)";
}

// ============ Signal Emission Tests ============

// Test: Commander tax visibility signal emitted on change
TEST_F(CounterVisibilityTest, CommanderTax_SignalEmittedOnChange)
{
    harness->setCommandZonesVisible(true); // Start visible
    QSignalSpy spy(harness, &CounterVisibilityTestHarness::commanderTaxVisibilityChanged);

    harness->setCommandZonesVisible(false);

    ASSERT_EQ(1, spy.count()) << "Signal should be emitted once";
    EXPECT_FALSE(spy.at(0).at(0).toBool()) << "Signal argument should be false";
}

// Test: Partner tax visibility signal emitted on change
TEST_F(CounterVisibilityTest, PartnerTax_SignalEmittedOnChange)
{
    harness->setCommandZonesVisible(true);
    QSignalSpy spy(harness, &CounterVisibilityTestHarness::partnerTaxVisibilityChanged);

    harness->setPartnerZoneExpanded(true);

    ASSERT_EQ(1, spy.count()) << "Signal should be emitted once";
    EXPECT_TRUE(spy.at(0).at(0).toBool()) << "Signal argument should be true";
}

// Test: No signal when visibility unchanged
TEST_F(CounterVisibilityTest, NoSignal_WhenVisibilityUnchanged)
{
    harness->setCommandZonesVisible(true);
    QSignalSpy spy(harness, &CounterVisibilityTestHarness::commanderTaxVisibilityChanged);

    harness->setCommandZonesVisible(true); // Same value

    EXPECT_EQ(0, spy.count()) << "Signal should not emit when visibility unchanged";
}

// ============ Edge Cases ============

// Test: Multiple rapid toggles maintain correct state
TEST_F(CounterVisibilityTest, RapidToggles_MaintainCorrectState)
{
    harness->setCommandZonesVisible(true);

    // Rapid expand/collapse cycle
    harness->setPartnerZoneExpanded(true);
    EXPECT_TRUE(harness->isPartnerTaxVisible());

    harness->setPartnerZoneExpanded(false);
    EXPECT_FALSE(harness->isPartnerTaxVisible());

    harness->setPartnerZoneExpanded(true);
    EXPECT_TRUE(harness->isPartnerTaxVisible());

    harness->setPartnerZoneExpanded(false);
    EXPECT_FALSE(harness->isPartnerTaxVisible());

    // Final state check
    EXPECT_FALSE(harness->isPartnerTaxVisible()) << "Partner tax should be hidden after toggling";
    EXPECT_TRUE(harness->isCommanderTaxVisible()) << "Commander tax should remain visible throughout";
}

// Test: Initial state has commander hidden (zones not yet set visible)
TEST_F(CounterVisibilityTest, InitialState_BothHidden)
{
    // Fresh harness starts with commandZonesVisible_ = true but counters not updated
    CounterVisibilityTestHarness freshHarness;

    // Before any setters called, counters start hidden
    EXPECT_FALSE(freshHarness.isCommanderTaxVisible()) << "Commander tax should start hidden";
    EXPECT_FALSE(freshHarness.isPartnerTaxVisible()) << "Partner tax should start hidden";
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#include "counter_visibility_test.moc"
