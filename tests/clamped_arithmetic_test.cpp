/** @file clamped_arithmetic_test.cpp
 *  @brief Tests for shared helpers in clamped_arithmetic.h.
 *  @ingroup Tests
 */

#include <gtest/gtest.h>
#include <libcockatrice/utility/clamped_arithmetic.h>
#include <limits>

TEST(AddClamped, AddsWithinBounds)
{
    EXPECT_EQ(addClamped(5, 3, 0, 100), 8);
    EXPECT_EQ(addClamped(10, -3, 0, 100), 7);
}

TEST(AddClamped, ClampsToUpperAndLowerBound)
{
    EXPECT_EQ(addClamped(99, 5, 0, 100), 100);  // saturates at max
    EXPECT_EQ(addClamped(2, -10, 0, 100), 0);   // saturates at min
    EXPECT_EQ(addClamped(999, 1, 0, 999), 999); // crossing the counter cap holds at the bound
}

TEST(AddClamped, IntOverflowDoesNotWrap)
{
    // The 64-bit intermediate must prevent signed-int overflow UB.
    constexpr int intMax = std::numeric_limits<int>::max();
    constexpr int intMin = std::numeric_limits<int>::min();
    EXPECT_EQ(addClamped(intMax, 1, intMin, intMax), intMax);
    EXPECT_EQ(addClamped(intMax, intMax, intMin, intMax), intMax);
}

TEST(AddClamped, IntUnderflowDoesNotWrap)
{
    constexpr int intMax = std::numeric_limits<int>::max();
    constexpr int intMin = std::numeric_limits<int>::min();
    EXPECT_EQ(addClamped(intMin, -1, intMin, intMax), intMin);
    EXPECT_EQ(addClamped(intMin, intMin, intMin, intMax), intMin);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
