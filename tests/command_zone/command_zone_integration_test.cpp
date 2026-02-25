/**
 * @file command_zone_integration_test.cpp
 * @brief Integration tests for CommandZone counter coordination
 *
 * Tests the interaction between command zones and tax counters:
 * - Counter parenting to correct zone
 * - Counter positioning follows zone movement
 * - Primary, partner, companion, and background zone coordination
 */

#include "command_zone_test_common.h"

#include <QPointF>
#include <QRectF>
#include <gtest/gtest.h>

/**
 * @brief Mock counter for testing parenting and positioning.
 *
 * Simulates CommanderTaxCounter without QGraphicsScene dependencies.
 * Tracks parent relationship and position for verification.
 */
class MockTaxCounter
{
    void *parentItem_;
    QPointF pos_;
    qreal zValue_;
    bool visible_;

public:
    explicit MockTaxCounter(void *parent = nullptr) : parentItem_(parent), pos_(0, 0), zValue_(0), visible_(true)
    {
    }

    void setParentItem(void *parent)
    {
        parentItem_ = parent;
    }

    [[nodiscard]] void *parentItem() const
    {
        return parentItem_;
    }

    void setPos(qreal x, qreal y)
    {
        pos_ = QPointF(x, y);
    }

    void setPos(const QPointF &p)
    {
        pos_ = p;
    }

    [[nodiscard]] QPointF pos() const
    {
        return pos_;
    }

    [[nodiscard]] QPointF scenePos() const
    {
        if (parentItem_) {
            return pos_;
        }
        return pos_;
    }

    void setZValue(qreal z)
    {
        zValue_ = z;
    }

    [[nodiscard]] qreal zValue() const
    {
        return zValue_;
    }

    void setVisible(bool v)
    {
        visible_ = v;
    }

    [[nodiscard]] bool isVisible() const
    {
        return visible_;
    }
};

/**
 * @brief Mock command zone for testing counter coordination.
 *
 * Simulates CommandZone state management without QGraphicsScene.
 * Supports all 4 zone types: Primary, Partner, Companion, Background.
 */
class MockCommandZone
{
    CommandZoneType zoneType_;
    ZoneVisibility visibility_;
    bool visible_;
    QPointF pos_;
    qreal height_;

public:
    explicit MockCommandZone(CommandZoneType zoneType, qreal height = 100.0)
        : zoneType_(zoneType),
          visibility_(zoneType == CommandZoneType::Primary ? ZoneVisibility::Expanded : ZoneVisibility::Collapsed),
          visible_(true), pos_(0, 0), height_(height)
    {
    }

    [[nodiscard]] bool isPrimary() const
    {
        return zoneType_ == CommandZoneType::Primary;
    }

    [[nodiscard]] CommandZoneType getZoneType() const
    {
        return zoneType_;
    }

    [[nodiscard]] bool isExpanded() const
    {
        return visibility_ != ZoneVisibility::Collapsed;
    }

    void setExpanded(bool e)
    {
        visibility_ = e ? ZoneVisibility::Expanded : ZoneVisibility::Collapsed;
    }

    [[nodiscard]] bool isMinimized() const
    {
        return visibility_ == ZoneVisibility::Minimized;
    }

    void setMinimized(bool m)
    {
        if (visibility_ != ZoneVisibility::Collapsed) {
            visibility_ = m ? ZoneVisibility::Minimized : ZoneVisibility::Expanded;
        }
    }

    [[nodiscard]] bool isVisible() const
    {
        return visible_;
    }

    void setVisible(bool v)
    {
        visible_ = v;
    }

    void setPos(const QPointF &p)
    {
        pos_ = p;
    }

    [[nodiscard]] QPointF pos() const
    {
        return pos_;
    }

    [[nodiscard]] qreal currentHeight() const
    {
        if (visibility_ == ZoneVisibility::Collapsed) {
            return 0;
        }
        return visibility_ == ZoneVisibility::Minimized ? (height_ * 0.25) : height_;
    }
};

/**
 * @brief Test harness for counter-zone integration logic.
 *
 * Extracts the counter positioning and visibility logic from
 * PlayerGraphicsItem::rearrangeCounters for isolated testing.
 *
 * @warning SYNC REQUIRED: This harness duplicates logic from
 * player_graphics_item.cpp. Update if implementation changes.
 */
class CounterZoneIntegrationHarness
{
public:
    MockCommandZone commandZone{CommandZoneType::Primary, 100.0};
    MockCommandZone partnerZone{CommandZoneType::Partner, 100.0};
    MockCommandZone companionZone{CommandZoneType::Companion, 100.0};
    MockCommandZone backgroundZone{CommandZoneType::Background, 100.0};
    MockTaxCounter commanderTaxCounter{nullptr};
    MockTaxCounter partnerTaxCounter{nullptr};

    static constexpr qreal TAX_COUNTERS_Z = 200000002.0;

    CounterZoneIntegrationHarness()
    {
    }

    void setupCounterParenting()
    {
        commanderTaxCounter.setParentItem(static_cast<void *>(&commandZone));
        partnerTaxCounter.setParentItem(static_cast<void *>(&partnerZone));
    }

    void rearrangeCounters()
    {
        bool commandZonesVisible = commandZone.isVisible();

        if (commandZonesVisible) {
            commanderTaxCounter.setPos(2, 2);
            commanderTaxCounter.setZValue(TAX_COUNTERS_Z);
            commanderTaxCounter.setVisible(true);
        } else {
            commanderTaxCounter.setVisible(false);
        }

        if (commandZonesVisible && partnerZone.isExpanded()) {
            partnerTaxCounter.setPos(2, 2);
            partnerTaxCounter.setZValue(TAX_COUNTERS_Z);
            partnerTaxCounter.setVisible(true);
        } else {
            partnerTaxCounter.setVisible(false);
        }
    }
};

class CommandZoneIntegrationTest : public ::testing::Test
{
protected:
    CounterZoneIntegrationHarness *harness;

    void SetUp() override
    {
        harness = new CounterZoneIntegrationHarness();
        harness->setupCounterParenting();
    }

    void TearDown() override
    {
        delete harness;
    }
};

// ============ Counter Parenting Tests ============

TEST_F(CommandZoneIntegrationTest, CommanderTaxCounter_ParentedToCommandZone)
{
    EXPECT_NE(nullptr, harness->commanderTaxCounter.parentItem()) << "Commander tax counter should have a parent";

    EXPECT_EQ(static_cast<void *>(&harness->commandZone), harness->commanderTaxCounter.parentItem())
        << "Commander tax counter should be parented to command zone";
}

TEST_F(CommandZoneIntegrationTest, PartnerTaxCounter_ParentedToPartnerZone)
{
    EXPECT_NE(nullptr, harness->partnerTaxCounter.parentItem()) << "Partner tax counter should have a parent";

    EXPECT_EQ(static_cast<void *>(&harness->partnerZone), harness->partnerTaxCounter.parentItem())
        << "Partner tax counter should be parented to partner zone";
}

// ============ Counter Positioning Tests ============

TEST_F(CommandZoneIntegrationTest, Counters_PositionedAtTopLeft)
{
    harness->commandZone.setVisible(true);
    harness->partnerZone.setExpanded(true);
    harness->rearrangeCounters();

    EXPECT_EQ(QPointF(2, 2), harness->commanderTaxCounter.pos())
        << "Commander tax counter should be at (2, 2) in command zone";

    EXPECT_EQ(QPointF(2, 2), harness->partnerTaxCounter.pos())
        << "Partner tax counter should be at (2, 2) in partner zone";
}

TEST_F(CommandZoneIntegrationTest, Counters_CorrectZValue)
{
    harness->commandZone.setVisible(true);
    harness->partnerZone.setExpanded(true);
    harness->rearrangeCounters();

    EXPECT_EQ(CounterZoneIntegrationHarness::TAX_COUNTERS_Z, harness->commanderTaxCounter.zValue())
        << "Commander tax counter should have TAX_COUNTERS z-value";

    EXPECT_EQ(CounterZoneIntegrationHarness::TAX_COUNTERS_Z, harness->partnerTaxCounter.zValue())
        << "Partner tax counter should have TAX_COUNTERS z-value";
}

// ============ Zone Type Tests ============

TEST_F(CommandZoneIntegrationTest, ZoneTypes_CorrectlyIdentified)
{
    EXPECT_EQ(CommandZoneType::Primary, harness->commandZone.getZoneType());
    EXPECT_EQ(CommandZoneType::Partner, harness->partnerZone.getZoneType());
    EXPECT_EQ(CommandZoneType::Companion, harness->companionZone.getZoneType());
    EXPECT_EQ(CommandZoneType::Background, harness->backgroundZone.getZoneType());
}

TEST_F(CommandZoneIntegrationTest, ZoneTypes_IsPrimaryCorrect)
{
    EXPECT_TRUE(harness->commandZone.isPrimary());
    EXPECT_FALSE(harness->partnerZone.isPrimary());
    EXPECT_FALSE(harness->companionZone.isPrimary());
    EXPECT_FALSE(harness->backgroundZone.isPrimary());
}

// ============ Zone Coordination Tests ============

TEST_F(CommandZoneIntegrationTest, ZoneInitialState)
{
    EXPECT_TRUE(harness->commandZone.isExpanded()) << "Command zone should start expanded";
    EXPECT_FALSE(harness->partnerZone.isExpanded()) << "Partner zone should start collapsed";
    EXPECT_FALSE(harness->companionZone.isExpanded()) << "Companion zone should start collapsed";
    EXPECT_FALSE(harness->backgroundZone.isExpanded()) << "Background zone should start collapsed";
}

TEST_F(CommandZoneIntegrationTest, PartnerZone_CollapsedHeight)
{
    EXPECT_FALSE(harness->partnerZone.isExpanded());
    EXPECT_EQ(0, harness->partnerZone.currentHeight()) << "Collapsed partner zone should have 0 height";
}

TEST_F(CommandZoneIntegrationTest, PartnerZone_ExpandedHeight)
{
    harness->partnerZone.setExpanded(true);
    EXPECT_EQ(100.0, harness->partnerZone.currentHeight()) << "Expanded partner zone should have full height";
}

TEST_F(CommandZoneIntegrationTest, CompanionZone_ExpandedHeight)
{
    harness->companionZone.setExpanded(true);
    EXPECT_EQ(100.0, harness->companionZone.currentHeight()) << "Expanded companion zone should have full height";
}

TEST_F(CommandZoneIntegrationTest, BackgroundZone_ExpandedHeight)
{
    harness->backgroundZone.setExpanded(true);
    EXPECT_EQ(100.0, harness->backgroundZone.currentHeight()) << "Expanded background zone should have full height";
}

TEST_F(CommandZoneIntegrationTest, Zone_MinimizedHeight)
{
    harness->commandZone.setMinimized(true);
    EXPECT_EQ(25.0, harness->commandZone.currentHeight()) << "Minimized zone should have 25% height";
}

// ============ Counter Visibility During Transitions ============

TEST_F(CommandZoneIntegrationTest, AllCountersVisible_WhenZonesExpanded)
{
    harness->commandZone.setVisible(true);
    harness->partnerZone.setExpanded(true);
    harness->rearrangeCounters();

    EXPECT_TRUE(harness->commanderTaxCounter.isVisible()) << "Commander tax counter should be visible";
    EXPECT_TRUE(harness->partnerTaxCounter.isVisible()) << "Partner tax counter should be visible";
}

TEST_F(CommandZoneIntegrationTest, PartnerCounter_HiddenWhenCollapsed)
{
    harness->commandZone.setVisible(true);
    harness->partnerZone.setExpanded(false);
    harness->rearrangeCounters();

    EXPECT_TRUE(harness->commanderTaxCounter.isVisible()) << "Commander tax counter should remain visible";
    EXPECT_FALSE(harness->partnerTaxCounter.isVisible()) << "Partner tax counter should be hidden when collapsed";
}

TEST_F(CommandZoneIntegrationTest, AllCountersHidden_WhenZonesNotVisible)
{
    harness->commandZone.setVisible(false);
    harness->rearrangeCounters();

    EXPECT_FALSE(harness->commanderTaxCounter.isVisible())
        << "Commander tax counter should be hidden when zones not visible";
    EXPECT_FALSE(harness->partnerTaxCounter.isVisible())
        << "Partner tax counter should be hidden when zones not visible";
}

TEST_F(CommandZoneIntegrationTest, CommanderCounter_VisibleWhenMinimized)
{
    harness->commandZone.setVisible(true);
    harness->commandZone.setMinimized(true);
    harness->rearrangeCounters();

    EXPECT_TRUE(harness->commanderTaxCounter.isVisible())
        << "Commander tax counter should remain visible when zone minimized";
}

TEST_F(CommandZoneIntegrationTest, VisibilityUpdates_DuringToggleCycle)
{
    harness->commandZone.setVisible(true);

    harness->partnerZone.setExpanded(false);
    harness->rearrangeCounters();
    EXPECT_FALSE(harness->partnerTaxCounter.isVisible()) << "Partner counter hidden when collapsed";

    harness->partnerZone.setExpanded(true);
    harness->rearrangeCounters();
    EXPECT_TRUE(harness->partnerTaxCounter.isVisible()) << "Partner counter visible when expanded";

    harness->partnerZone.setExpanded(false);
    harness->rearrangeCounters();
    EXPECT_FALSE(harness->partnerTaxCounter.isVisible()) << "Partner counter hidden after collapse";
}

// ============ Companion/Background Zone Tests (No Tax Counters) ============

TEST_F(CommandZoneIntegrationTest, CompanionZone_NoTaxCounter)
{
    // Companion zone has no tax counter - this test verifies zone behavior independently
    harness->companionZone.setExpanded(true);
    EXPECT_TRUE(harness->companionZone.isExpanded());
    EXPECT_EQ(100.0, harness->companionZone.currentHeight());
}

TEST_F(CommandZoneIntegrationTest, BackgroundZone_NoTaxCounter)
{
    // Background zone has no tax counter - this test verifies zone behavior independently
    harness->backgroundZone.setExpanded(true);
    EXPECT_TRUE(harness->backgroundZone.isExpanded());
    EXPECT_EQ(100.0, harness->backgroundZone.currentHeight());
}

// ============ Zone Positioning Tests ============

/**
 * @brief Test harness for zone positioning calculation.
 *
 * Uses a HYBRID architecture matching the actual implementation:
 * - Partner Zone: Qt child of Command Zone (uses relative coordinates)
 * - Companion/Background Zones: Qt siblings of Command Zone (use absolute coordinates)
 *
 * This distinction matters because Qt transforms work differently for children vs siblings.
 */
class ZonePositioningHarness
{
public:
    MockCommandZone commandZone{CommandZoneType::Primary, 100.0};
    MockCommandZone partnerZone{CommandZoneType::Partner, 100.0};
    MockCommandZone companionZone{CommandZoneType::Companion, 100.0};
    MockCommandZone backgroundZone{CommandZoneType::Background, 100.0};

    /**
     * @brief Calculates scene position for a zone in the parent chain.
     *
     * Command zone position is the base. Each subsequent zone is offset
     * by the cumulative height of all ancestor zones.
     */
    QPointF scenePositionOf(const MockCommandZone &zone) const
    {
        QPointF base = commandZone.pos();

        if (&zone == &commandZone) {
            return base;
        }

        qreal offset = commandZone.currentHeight();
        if (&zone == &partnerZone) {
            return base + QPointF(0, offset);
        }

        offset += partnerZone.currentHeight();
        if (&zone == &companionZone) {
            return base + QPointF(0, offset);
        }

        offset += companionZone.currentHeight();
        if (&zone == &backgroundZone) {
            return base + QPointF(0, offset);
        }

        return base;
    }

    /**
     * @brief Calculates the actual position for a zone based on hybrid architecture.
     *
     * - Partner Zone: Returns relative position (Qt child of Command Zone)
     * - Companion/Background: Returns absolute position (Qt siblings)
     */
    QPointF positionFor(const MockCommandZone &zone) const
    {
        QPointF base = commandZone.pos();

        if (&zone == &commandZone) {
            return base;
        }
        if (&zone == &partnerZone) {
            return QPointF(0, commandZone.currentHeight()); // Relative (Qt child)
        }

        // Companion and Background use ABSOLUTE positioning (Qt siblings)
        qreal runningY = commandZone.currentHeight() + partnerZone.currentHeight();
        if (&zone == &companionZone) {
            return QPointF(base.x(), base.y() + runningY);
        }

        runningY += companionZone.currentHeight();
        if (&zone == &backgroundZone) {
            return QPointF(base.x(), base.y() + runningY);
        }

        return QPointF(0, 0);
    }

    /**
     * @brief Calculates total zone height for stack zone positioning.
     *
     * This matches the visibility-aware logic in PlayerGraphicsItem::totalCommandZoneHeight().
     * Command zone and partner use Qt parent-child relationship, so checking command visibility
     * implicitly covers partner (Qt hides children when parent is hidden).
     * Companion and background are Qt siblings with independent visibility.
     */
    qreal totalHeight() const
    {
        qreal total = 0;

        // Command zone and partner: Qt parent-child relationship
        if (commandZone.isVisible()) {
            total += commandZone.currentHeight();
            if (partnerZone.isExpanded()) {
                total += partnerZone.currentHeight();
            }
        }

        // Sibling zones: independent visibility
        if (companionZone.isVisible()) {
            total += companionZone.currentHeight();
        }

        if (backgroundZone.isVisible()) {
            total += backgroundZone.currentHeight();
        }

        return total;
    }

    /**
     * @brief Legacy total height calculation (non-visibility-aware).
     *
     * Used for tests that need the original behavior for comparison.
     */
    qreal totalHeightIgnoringVisibility() const
    {
        return commandZone.currentHeight() + partnerZone.currentHeight() + companionZone.currentHeight() +
               backgroundZone.currentHeight();
    }
};

class ZonePositioningTest : public ::testing::Test
{
protected:
    ZonePositioningHarness harness;
};

TEST_F(ZonePositioningTest, AllZonesExpanded_CorrectScenePositions)
{
    harness.commandZone.setPos(QPointF(50, 0));
    harness.partnerZone.setExpanded(true);
    harness.companionZone.setExpanded(true);
    harness.backgroundZone.setExpanded(true);

    EXPECT_EQ(QPointF(50, 0), harness.scenePositionOf(harness.commandZone));
    EXPECT_EQ(QPointF(50, 100), harness.scenePositionOf(harness.partnerZone));
    EXPECT_EQ(QPointF(50, 200), harness.scenePositionOf(harness.companionZone));
    EXPECT_EQ(QPointF(50, 300), harness.scenePositionOf(harness.backgroundZone));
}

TEST_F(ZonePositioningTest, AllZonesExpanded_CorrectPositions)
{
    harness.commandZone.setPos(QPointF(0, 0));
    harness.partnerZone.setExpanded(true);
    harness.companionZone.setExpanded(true);
    harness.backgroundZone.setExpanded(true);

    // Partner: relative position (Qt child of Command)
    EXPECT_EQ(QPointF(0, 100), harness.positionFor(harness.partnerZone));
    // Companion: absolute position at (0, 200) - base + command height + partner height
    EXPECT_EQ(QPointF(0, 200), harness.positionFor(harness.companionZone));
    // Background: absolute position at (0, 300) - base + command + partner + companion
    EXPECT_EQ(QPointF(0, 300), harness.positionFor(harness.backgroundZone));
}

TEST_F(ZonePositioningTest, PartnerCollapsed_CompanionAtCommandBottom)
{
    harness.commandZone.setPos(QPointF(50, 0));
    harness.partnerZone.setExpanded(false);
    harness.companionZone.setExpanded(true);
    harness.backgroundZone.setExpanded(true);

    EXPECT_EQ(0, harness.partnerZone.currentHeight()) << "Collapsed partner should have 0 height";
    EXPECT_EQ(QPointF(50, 100), harness.scenePositionOf(harness.companionZone))
        << "Companion should be at command bottom when partner is collapsed";
    EXPECT_EQ(QPointF(50, 200), harness.scenePositionOf(harness.backgroundZone));
}

TEST_F(ZonePositioningTest, PartnerCollapsed_CompanionPosition)
{
    harness.commandZone.setPos(QPointF(0, 0));
    harness.partnerZone.setExpanded(false);
    harness.companionZone.setExpanded(true);

    // Companion: absolute position = base + command height + partner height (0)
    EXPECT_EQ(QPointF(0, 100), harness.positionFor(harness.companionZone))
        << "Companion at (0, 100) when partner collapsed (0 + 100 + 0)";
}

TEST_F(ZonePositioningTest, AllCollapsed_TotalHeightIsCommandOnly)
{
    harness.partnerZone.setExpanded(false);
    harness.companionZone.setExpanded(false);
    harness.backgroundZone.setExpanded(false);

    EXPECT_EQ(100, harness.totalHeight()) << "Only command zone height when others collapsed";
}

TEST_F(ZonePositioningTest, AllExpanded_TotalHeightIs400)
{
    harness.partnerZone.setExpanded(true);
    harness.companionZone.setExpanded(true);
    harness.backgroundZone.setExpanded(true);

    EXPECT_EQ(400, harness.totalHeight()) << "4 zones × 100 = 400";
}

TEST_F(ZonePositioningTest, MixedStates_CorrectTotalHeight)
{
    harness.partnerZone.setExpanded(true);
    harness.partnerZone.setMinimized(true);
    harness.companionZone.setExpanded(true);
    harness.backgroundZone.setExpanded(false);

    EXPECT_EQ(100 + 25 + 100 + 0, harness.totalHeight())
        << "Command (100) + Partner minimized (25) + Companion (100) + Background collapsed (0)";
}

// ============ Position Verification After State Changes ============

TEST_F(ZonePositioningTest, MinimizeCommand_CompanionMovesUp)
{
    harness.commandZone.setPos(QPointF(50, 0));
    harness.partnerZone.setExpanded(true);
    harness.companionZone.setExpanded(true);

    EXPECT_EQ(QPointF(50, 200), harness.scenePositionOf(harness.companionZone));

    harness.commandZone.setMinimized(true);

    EXPECT_EQ(QPointF(50, 125), harness.scenePositionOf(harness.companionZone))
        << "Companion should move up when command zone minimizes (25 + 100 = 125)";
}

TEST_F(ZonePositioningTest, CollapsePartner_CompanionMovesUp)
{
    harness.commandZone.setPos(QPointF(50, 0));
    harness.partnerZone.setExpanded(true);
    harness.companionZone.setExpanded(true);

    EXPECT_EQ(QPointF(50, 200), harness.scenePositionOf(harness.companionZone));

    harness.partnerZone.setExpanded(false);

    EXPECT_EQ(QPointF(50, 100), harness.scenePositionOf(harness.companionZone))
        << "Companion should move up when partner collapses (100 + 0 = 100)";
}

// ============ Visibility-Aware totalHeight Tests ============

TEST_F(ZonePositioningTest, CommandZoneHidden_HeightExcludesCommandAndPartner)
{
    harness.commandZone.setVisible(false);
    harness.partnerZone.setExpanded(true);
    harness.companionZone.setExpanded(true);
    harness.backgroundZone.setExpanded(true);

    EXPECT_EQ(200, harness.totalHeight()) << "Only companion (100) + background (100) when command zone hidden";
}

TEST_F(ZonePositioningTest, CommandZoneHidden_PartnerExpandedIgnored)
{
    harness.commandZone.setVisible(false);
    harness.partnerZone.setExpanded(true);

    EXPECT_EQ(0, harness.totalHeight())
        << "Partner height ignored when command zone is Qt-hidden (Qt cascades visibility to children)";
}

TEST_F(ZonePositioningTest, OnlyCompanionVisible_HeightIsCompanionOnly)
{
    harness.commandZone.setVisible(false);
    harness.companionZone.setVisible(true);
    harness.companionZone.setExpanded(true);
    harness.backgroundZone.setVisible(false);

    EXPECT_EQ(100, harness.totalHeight()) << "Only companion height (100) when it's the only visible zone";
}

TEST_F(ZonePositioningTest, OnlyBackgroundVisible_HeightIsBackgroundOnly)
{
    harness.commandZone.setVisible(false);
    harness.companionZone.setVisible(false);
    harness.backgroundZone.setVisible(true);
    harness.backgroundZone.setExpanded(true);

    EXPECT_EQ(100, harness.totalHeight()) << "Only background height (100) when it's the only visible zone";
}

TEST_F(ZonePositioningTest, CompanionAndBackgroundVisible_HeightIsBoth)
{
    harness.commandZone.setVisible(false);
    harness.companionZone.setVisible(true);
    harness.companionZone.setExpanded(true);
    harness.backgroundZone.setVisible(true);
    harness.backgroundZone.setExpanded(true);

    EXPECT_EQ(200, harness.totalHeight()) << "Companion (100) + Background (100) when both visible, command hidden";
}

TEST_F(ZonePositioningTest, AllZonesVisible_HeightIncludesAll)
{
    harness.commandZone.setVisible(true);
    harness.partnerZone.setExpanded(true);
    harness.companionZone.setVisible(true);
    harness.companionZone.setExpanded(true);
    harness.backgroundZone.setVisible(true);
    harness.backgroundZone.setExpanded(true);

    EXPECT_EQ(400, harness.totalHeight()) << "All zones visible and expanded: 4 × 100 = 400";
}

TEST_F(ZonePositioningTest, CompanionCollapsed_NoHeightContribution)
{
    harness.commandZone.setVisible(false);
    harness.companionZone.setVisible(true);
    harness.companionZone.setExpanded(false);
    harness.backgroundZone.setVisible(true);
    harness.backgroundZone.setExpanded(true);

    EXPECT_EQ(100, harness.totalHeight()) << "Only background (100) contributes when companion is collapsed";
}

TEST_F(ZonePositioningTest, CompanionMinimized_ReducedHeightContribution)
{
    harness.commandZone.setVisible(false);
    harness.companionZone.setVisible(true);
    harness.companionZone.setExpanded(true);
    harness.companionZone.setMinimized(true);
    harness.backgroundZone.setVisible(false);

    EXPECT_EQ(25, harness.totalHeight()) << "Minimized companion contributes 25% height (25)";
}

// ============ Toggle Visibility Tests ============

/**
 * @brief Test harness for toggle visibility logic.
 *
 * Simulates the toggle visibility rules from positionCommandZones()
 * to test behavior when intermediate zones are disabled.
 *
 * @warning SYNC REQUIRED: This harness duplicates logic from
 * player_graphics_item.cpp. Update if implementation changes.
 */
class ToggleVisibilityHarness
{
public:
    MockCommandZone commandZone{CommandZoneType::Primary, 100.0};
    MockCommandZone partnerZone{CommandZoneType::Partner, 100.0};
    MockCommandZone companionZone{CommandZoneType::Companion, 100.0};
    MockCommandZone backgroundZone{CommandZoneType::Background, 100.0};

    bool partnerToggleVisible = false;
    bool companionToggleVisible = false;
    bool backgroundToggleVisible = false;

    /**
     * @brief Calculates toggle visibility using the same logic as positionCommandZones().
     */
    void updateToggleVisibility()
    {
        bool commandZoneEnabled = commandZone.isVisible();
        bool companionZoneEnabled = companionZone.isVisible();
        bool backgroundZoneEnabled = backgroundZone.isVisible();

        if (!commandZoneEnabled && !companionZoneEnabled && !backgroundZoneEnabled) {
            partnerToggleVisible = false;
            companionToggleVisible = false;
            backgroundToggleVisible = false;
            return;
        }

        bool partnerCollapsed = !partnerZone.isExpanded();
        bool companionCollapsed = !companionZone.isExpanded();
        bool backgroundCollapsed = !backgroundZone.isExpanded();

        bool partnerIsOpen = !partnerCollapsed;
        bool companionIsOpen = !companionCollapsed;
        bool backgroundIsOpen = !backgroundCollapsed;
        bool noDeeperZonesOpen = companionCollapsed && backgroundCollapsed;

        partnerToggleVisible = commandZoneEnabled && (partnerIsOpen || noDeeperZonesOpen);

        companionToggleVisible = companionZoneEnabled && (companionIsOpen || (partnerIsOpen && backgroundCollapsed) ||
                                                          (!commandZoneEnabled && noDeeperZonesOpen));

        backgroundToggleVisible =
            backgroundZoneEnabled && (backgroundIsOpen || companionIsOpen || (partnerIsOpen && !companionZoneEnabled) ||
                                      (!commandZoneEnabled && !companionZoneEnabled));
    }
};

class ToggleVisibilityTest : public ::testing::Test
{
protected:
    ToggleVisibilityHarness harness;

    void SetUp() override
    {
        harness.commandZone.setVisible(true);
        harness.partnerZone.setVisible(true);
        harness.companionZone.setVisible(true);
        harness.backgroundZone.setVisible(true);
    }
};

// Original bug: Companion disabled, command zone enabled
TEST_F(ToggleVisibilityTest, CompanionDisabled_PartnerOpens_BackgroundToggleAppears)
{
    harness.companionZone.setVisible(false);

    harness.updateToggleVisibility();
    EXPECT_TRUE(harness.partnerToggleVisible) << "Partner toggle at game start";

    harness.partnerZone.setExpanded(true);
    harness.updateToggleVisibility();

    EXPECT_TRUE(harness.backgroundToggleVisible)
        << "Background toggle should appear when partner opens (skipping disabled companion)";
}

// Bug 2: Command zone disabled, companion enabled
TEST_F(ToggleVisibilityTest, CommandZoneDisabled_CompanionEnabled_CompanionIsEntryPoint)
{
    harness.commandZone.setVisible(false);

    harness.updateToggleVisibility();

    EXPECT_FALSE(harness.partnerToggleVisible) << "Partner toggle hidden when command zone disabled";
    EXPECT_TRUE(harness.companionToggleVisible) << "Companion toggle should be entry point when command zone disabled";
}

// Edge case: Only background enabled
TEST_F(ToggleVisibilityTest, OnlyBackgroundEnabled_BackgroundIsEntryPoint)
{
    harness.commandZone.setVisible(false);
    harness.companionZone.setVisible(false);

    harness.updateToggleVisibility();

    EXPECT_FALSE(harness.partnerToggleVisible);
    EXPECT_FALSE(harness.companionToggleVisible);
    EXPECT_TRUE(harness.backgroundToggleVisible)
        << "Background toggle should be entry point when it's the only zone enabled";
}

// Regression: All zones enabled, normal cascade
TEST_F(ToggleVisibilityTest, AllEnabled_NormalCascade)
{
    harness.updateToggleVisibility();
    EXPECT_TRUE(harness.partnerToggleVisible) << "Partner toggle at game start";
    EXPECT_FALSE(harness.companionToggleVisible) << "Companion toggle hidden at start";
    EXPECT_FALSE(harness.backgroundToggleVisible) << "Background toggle hidden at start";

    harness.partnerZone.setExpanded(true);
    harness.updateToggleVisibility();
    EXPECT_TRUE(harness.companionToggleVisible) << "Companion toggle after partner opens";
    EXPECT_FALSE(harness.backgroundToggleVisible) << "Background still hidden";

    harness.companionZone.setExpanded(true);
    harness.updateToggleVisibility();
    EXPECT_TRUE(harness.backgroundToggleVisible) << "Background toggle after companion opens";
}

// Edge case: Partner opens but all deeper zones disabled (dead-end cascade)
TEST_F(ToggleVisibilityTest, AllDeeperZonesDisabled_PartnerOpens_NoFurtherToggle)
{
    harness.companionZone.setVisible(false);
    harness.backgroundZone.setVisible(false);

    harness.partnerZone.setExpanded(true);
    harness.updateToggleVisibility();

    EXPECT_TRUE(harness.partnerToggleVisible) << "Partner toggle visible (open)";
    EXPECT_FALSE(harness.companionToggleVisible) << "Companion disabled";
    EXPECT_FALSE(harness.backgroundToggleVisible) << "Background disabled";
}

// Verify toggle stays visible when zone is open
TEST_F(ToggleVisibilityTest, OpenZone_ToggleRemainsVisible)
{
    harness.partnerZone.setExpanded(true);
    harness.companionZone.setExpanded(true);
    harness.backgroundZone.setExpanded(true);

    harness.updateToggleVisibility();

    EXPECT_TRUE(harness.partnerToggleVisible) << "Partner toggle visible when open";
    EXPECT_TRUE(harness.companionToggleVisible) << "Companion toggle visible when open";
    EXPECT_TRUE(harness.backgroundToggleVisible) << "Background toggle visible when open";
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
