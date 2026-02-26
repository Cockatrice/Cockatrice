/**
 * @file commander_tax_counter_test.cpp
 * @brief Unit tests for CommanderTaxCounter::setValue
 *
 * Tests the value clamping behavior for commander tax:
 * - Positive values pass through unchanged
 * - Zero values pass through unchanged
 * - Negative values are clamped to 0
 */

#include <QRectF>
#include <algorithm>
#include <climits>
#include <gtest/gtest.h>

/**
 * @brief Testable implementation of CommanderTaxCounter value handling.
 *
 * Extracts the setValue and boundingRect logic for unit testing without
 * requiring Player, ThemeManager, and QGraphicsScene dependencies.
 */
class CommanderTaxCounterTestHarness
{
    int value;
    int size;

public:
    explicit CommanderTaxCounterTestHarness(int _size = 24, int initialValue = 0) : value(initialValue), size(_size)
    {
    }

    /**
     * @brief Implementation of CommanderTaxCounter::setValue logic.
     *
     * Clamps negative values to 0, matching the real implementation:
     * AbstractCounter::setValue(qMax(0, _value));
     */
    void setValue(int _value)
    {
        // Clamp to 0 - commander tax cannot be negative
        value = std::max(0, _value);
    }

    [[nodiscard]] int getValue() const
    {
        return value;
    }

    /**
     * @brief Implementation of CommanderTaxCounter::boundingRect logic.
     *
     * Returns a square QRectF based on the counter size.
     */
    [[nodiscard]] QRectF boundingRect() const
    {
        return QRectF(0, 0, size, size);
    }

    [[nodiscard]] int getSize() const
    {
        return size;
    }
};

class CommanderTaxCounterTest : public ::testing::Test
{
protected:
    CommanderTaxCounterTestHarness *counter;

    void SetUp() override
    {
        counter = new CommanderTaxCounterTestHarness(24, 0);
    }

    void TearDown() override
    {
        delete counter;
    }
};

// Test: Positive value is stored correctly
TEST_F(CommanderTaxCounterTest, SetValue_Positive)
{
    counter->setValue(5);

    EXPECT_EQ(5, counter->getValue()) << "Positive value should be stored unchanged";
}

// Test: Zero value is stored correctly
TEST_F(CommanderTaxCounterTest, SetValue_Zero)
{
    counter->setValue(10); // First set to non-zero
    counter->setValue(0);

    EXPECT_EQ(0, counter->getValue()) << "Zero value should be stored unchanged";
}

// Test: Negative value is clamped to zero
TEST_F(CommanderTaxCounterTest, SetValue_Negative_ClampedToZero)
{
    counter->setValue(-5);

    EXPECT_EQ(0, counter->getValue()) << "Negative value should be clamped to 0";
}

// Test: Large negative value is clamped to zero
TEST_F(CommanderTaxCounterTest, SetValue_LargeNegative_ClampedToZero)
{
    counter->setValue(-1000);

    EXPECT_EQ(0, counter->getValue()) << "Large negative value should be clamped to 0";
}

// Test: Boundary condition - negative one
TEST_F(CommanderTaxCounterTest, SetValue_NegativeOne_ClampedToZero)
{
    counter->setValue(-1);

    EXPECT_EQ(0, counter->getValue()) << "-1 should be clamped to 0";
}

// Test: Large positive value is stored correctly
TEST_F(CommanderTaxCounterTest, SetValue_LargePositive)
{
    counter->setValue(100);

    EXPECT_EQ(100, counter->getValue()) << "Large positive value should be stored unchanged";
}

// Test: Multiple setValue calls work correctly
TEST_F(CommanderTaxCounterTest, SetValue_MultipleCalls)
{
    counter->setValue(5);
    EXPECT_EQ(5, counter->getValue());

    counter->setValue(10);
    EXPECT_EQ(10, counter->getValue());

    counter->setValue(-3);
    EXPECT_EQ(0, counter->getValue());

    counter->setValue(7);
    EXPECT_EQ(7, counter->getValue());
}

// Test: BoundingRect returns correct size
TEST_F(CommanderTaxCounterTest, BoundingRect_CorrectSize)
{
    QRectF rect = counter->boundingRect();

    EXPECT_EQ(0, rect.x()) << "BoundingRect x should be 0";
    EXPECT_EQ(0, rect.y()) << "BoundingRect y should be 0";
    EXPECT_EQ(24, rect.width()) << "BoundingRect width should equal size";
    EXPECT_EQ(24, rect.height()) << "BoundingRect height should equal size";
}

// Test: BoundingRect with different size
TEST_F(CommanderTaxCounterTest, BoundingRect_DifferentSize)
{
    delete counter;
    counter = new CommanderTaxCounterTestHarness(36, 0);

    QRectF rect = counter->boundingRect();

    EXPECT_EQ(36, rect.width()) << "BoundingRect width should match constructor size";
    EXPECT_EQ(36, rect.height()) << "BoundingRect height should match constructor size";
}

// Test: Initial value from constructor
TEST_F(CommanderTaxCounterTest, Constructor_InitialValue)
{
    delete counter;
    counter = new CommanderTaxCounterTestHarness(24, 5);

    EXPECT_EQ(5, counter->getValue()) << "Initial value from constructor should be set";
}

// Test: INT_MIN is clamped to zero
TEST_F(CommanderTaxCounterTest, SetValue_IntMin_ClampedToZero)
{
    counter->setValue(INT_MIN);

    EXPECT_EQ(0, counter->getValue()) << "INT_MIN should be clamped to 0";
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
