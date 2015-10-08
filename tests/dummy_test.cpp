#include "gtest/gtest.h"

namespace {
    class FooTest : public ::testing::Test {

    };

    TEST(DummyTest, Works) {
        ASSERT_EQ(1, 1) << "One is not equal to one";
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}