#include "gtest/gtest.h"
#include "../common/expression.h"

#include <cmath>

#define TEST_EXPR(name,a,b) TEST(name, Works) {  \
    Expression exp(8); \
	ASSERT_EQ(exp.parse(a), b) << a; \
}

namespace
{

	TEST_EXPR(Number, "1", 1)
	TEST_EXPR(Multiply, "2*2", 4)
	TEST_EXPR(Whitespace, "3 * 3", 9)
	TEST_EXPR(Powers, "2^8", 256)
	TEST_EXPR(OrderOfOperations, "2+2*2", 6)
	TEST_EXPR(Fn, "2*cos(1)", 2*cos(1))
	TEST_EXPR(Variable, "x / 2", 4)
	TEST_EXPR(Negative, "-2 * 2", -4)
	TEST_EXPR(UnknownFnReturnsZero, "blah(22)", 0)

} // namespace

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}