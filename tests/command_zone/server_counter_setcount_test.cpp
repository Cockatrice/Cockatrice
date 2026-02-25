/**
 * @file server_counter_setcount_test.cpp
 * @brief Unit tests for Server_Counter::setCount() return value behavior
 *
 * Tests the change-detection contract:
 * - Returns false when value unchanged (same value, clamped to same value)
 * - Returns true when value actually changes (including when clamping occurs)
 *
 * This ensures that Event_SetCounter is only emitted when the counter
 * value genuinely changes, preventing spurious log messages.
 */

#include <algorithm>
#include <gtest/gtest.h>
#include <limits>

/**
 * @brief Testable implementation of Server_Counter::setCount() logic.
 *
 * Extracts the setCount logic for unit testing without requiring
 * full Qt and server dependencies. Uses std::clamp to mirror qBound.
 *
 * @warning SYNC REQUIRED: This harness duplicates state logic from
 * Server_Counter (server_counter.h). If that implementation changes,
 * update this harness to match.
 */
class ServerCounterTestHarness
{
    int count;
    int minValue;
    int maxValue;

public:
    explicit ServerCounterTestHarness(int _count = 0,
                                      int _minValue = std::numeric_limits<int>::min(),
                                      int _maxValue = 9999)
        : count(_count), minValue(_minValue), maxValue(_maxValue)
    {
    }

    /**
     * @brief Sets the counter value with bounds checking.
     * @param _count The requested value
     * @return true if the value actually changed, false otherwise
     *
     * Mirrors Server_Counter::setCount() exactly.
     */
    bool setCount(int _count)
    {
        int oldCount = count;
        count = std::clamp(_count, minValue, maxValue);
        return count != oldCount;
    }

    [[nodiscard]] int getCount() const
    {
        return count;
    }

    [[nodiscard]] int getMinValue() const
    {
        return minValue;
    }

    [[nodiscard]] int getMaxValue() const
    {
        return maxValue;
    }
};

class ServerCounterSetCountTest : public ::testing::Test
{
protected:
    ServerCounterTestHarness *counter;

    void SetUp() override
    {
        counter = nullptr;
    }

    void TearDown() override
    {
        delete counter;
    }

    void createCounterWithMinMax(int initial = 0, int minVal = 0, int maxVal = 9999)
    {
        delete counter;
        counter = new ServerCounterTestHarness(initial, minVal, maxVal);
    }

    void createCommanderTaxCounter(int initial = 0)
    {
        createCounterWithMinMax(initial, 0, 9999);
    }
};

// ============ Return Value Tests: Value Unchanged ============

TEST_F(ServerCounterSetCountTest, SetCount_ReturnsFalse_WhenClampedAtMin)
{
    createCommanderTaxCounter(0);

    bool result = counter->setCount(-5);

    EXPECT_FALSE(result) << "setCount should return false when clamped to same value at min";
    EXPECT_EQ(0, counter->getCount()) << "Value should remain at min bound";
}

TEST_F(ServerCounterSetCountTest, SetCount_ReturnsFalse_WhenClampedAtMax)
{
    createCommanderTaxCounter(9999);

    bool result = counter->setCount(10000);

    EXPECT_FALSE(result) << "setCount should return false when clamped to same value at max";
    EXPECT_EQ(9999, counter->getCount()) << "Value should remain at max bound";
}

TEST_F(ServerCounterSetCountTest, SetCount_ReturnsFalse_WhenSettingToSameValue)
{
    createCommanderTaxCounter(5);

    bool result = counter->setCount(5);

    EXPECT_FALSE(result) << "setCount should return false when setting to same value";
    EXPECT_EQ(5, counter->getCount()) << "Value should remain unchanged";
}

TEST_F(ServerCounterSetCountTest, SetCount_ReturnsFalse_WhenAlreadyAtMinAndDecrementingMore)
{
    createCommanderTaxCounter(0);

    bool result = counter->setCount(-1000);

    EXPECT_FALSE(result) << "setCount should return false when already at min and trying to go lower";
    EXPECT_EQ(0, counter->getCount()) << "Value should stay at min";
}

TEST_F(ServerCounterSetCountTest, SetCount_ReturnsFalse_WhenAlreadyAtMaxAndIncrementingMore)
{
    createCommanderTaxCounter(9999);

    bool result = counter->setCount(99999);

    EXPECT_FALSE(result) << "setCount should return false when already at max and trying to go higher";
    EXPECT_EQ(9999, counter->getCount()) << "Value should stay at max";
}

// ============ Return Value Tests: Value Changed ============

TEST_F(ServerCounterSetCountTest, SetCount_ReturnsTrue_WhenValueChanges)
{
    createCommanderTaxCounter(0);

    bool result = counter->setCount(1);

    EXPECT_TRUE(result) << "setCount should return true when value changes";
    EXPECT_EQ(1, counter->getCount()) << "Value should be updated";
}

TEST_F(ServerCounterSetCountTest, SetCount_ReturnsTrue_WhenValueChangesWithClamping)
{
    createCommanderTaxCounter(5);

    bool result = counter->setCount(-100);

    EXPECT_TRUE(result) << "setCount should return true when value changes even if clamped";
    EXPECT_EQ(0, counter->getCount()) << "Value should be clamped to min";
}

TEST_F(ServerCounterSetCountTest, SetCount_ReturnsTrue_WhenValueDecrements)
{
    createCommanderTaxCounter(5);

    bool result = counter->setCount(4);

    EXPECT_TRUE(result) << "setCount should return true when value decrements";
    EXPECT_EQ(4, counter->getCount()) << "Value should be decremented";
}

TEST_F(ServerCounterSetCountTest, SetCount_ReturnsTrue_WhenGoingToMin)
{
    createCommanderTaxCounter(1);

    bool result = counter->setCount(0);

    EXPECT_TRUE(result) << "setCount should return true when reaching min from above";
    EXPECT_EQ(0, counter->getCount()) << "Value should be at min";
}

TEST_F(ServerCounterSetCountTest, SetCount_ReturnsTrue_WhenGoingToMax)
{
    createCommanderTaxCounter(9998);

    bool result = counter->setCount(9999);

    EXPECT_TRUE(result) << "setCount should return true when reaching max from below";
    EXPECT_EQ(9999, counter->getCount()) << "Value should be at max";
}

TEST_F(ServerCounterSetCountTest, SetCount_ReturnsTrue_WhenValueChangesToMaxWithClamping)
{
    createCommanderTaxCounter(100);

    bool result = counter->setCount(100000);

    EXPECT_TRUE(result) << "setCount should return true when value changes to max via clamping";
    EXPECT_EQ(9999, counter->getCount()) << "Value should be clamped to max";
}

// ============ Command Handler Simulation Tests ============
// These simulate the cmdIncCounter/cmdSetCounter behavior

TEST_F(ServerCounterSetCountTest, IncrementAtMin_EventWouldBeEmitted)
{
    createCommanderTaxCounter(0);

    bool didChange = counter->setCount(counter->getCount() + 1);

    EXPECT_TRUE(didChange) << "Increment from 0 to 1 should emit event";
}

TEST_F(ServerCounterSetCountTest, DecrementAtMin_NoEventEmitted)
{
    createCommanderTaxCounter(0);

    bool didChange = counter->setCount(counter->getCount() - 1);

    EXPECT_FALSE(didChange) << "Decrement at 0 should NOT emit event";
}

TEST_F(ServerCounterSetCountTest, IncrementAtMax_NoEventEmitted)
{
    createCommanderTaxCounter(9999);

    bool didChange = counter->setCount(counter->getCount() + 1);

    EXPECT_FALSE(didChange) << "Increment at max should NOT emit event";
}

TEST_F(ServerCounterSetCountTest, DecrementAtMax_EventWouldBeEmitted)
{
    createCommanderTaxCounter(9999);

    bool didChange = counter->setCount(counter->getCount() - 1);

    EXPECT_TRUE(didChange) << "Decrement from max should emit event";
}

TEST_F(ServerCounterSetCountTest, SetToSameValue_NoEventEmitted)
{
    createCommanderTaxCounter(5);

    bool didChange = counter->setCount(5);

    EXPECT_FALSE(didChange) << "Setting to same value should NOT emit event";
}

TEST_F(ServerCounterSetCountTest, SetToDifferentValue_EventWouldBeEmitted)
{
    createCommanderTaxCounter(5);

    bool didChange = counter->setCount(10);

    EXPECT_TRUE(didChange) << "Setting to different value should emit event";
}

// ============ Edge Cases ============

TEST_F(ServerCounterSetCountTest, CounterWithNegativeMinValue)
{
    createCounterWithMinMax(0, -10, 10);

    bool result = counter->setCount(-5);

    EXPECT_TRUE(result) << "Going to -5 from 0 with min=-10 should change";
    EXPECT_EQ(-5, counter->getCount());
}

TEST_F(ServerCounterSetCountTest, CounterWithNegativeMinValue_ClampedAtMin)
{
    createCounterWithMinMax(-10, -10, 10);

    bool result = counter->setCount(-20);

    EXPECT_FALSE(result) << "Already at min, should not change";
    EXPECT_EQ(-10, counter->getCount());
}

TEST_F(ServerCounterSetCountTest, ZeroRangeCounter_CannotChange)
{
    createCounterWithMinMax(5, 5, 5);

    bool result = counter->setCount(10);

    EXPECT_FALSE(result) << "Counter with min=max cannot change";
    EXPECT_EQ(5, counter->getCount());
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
