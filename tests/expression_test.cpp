#include "gtest/gtest.h"
#include "../common/expression.h"

#include <cmath>

#define TEST_EXPR(name,a,b) TEST(name, Works) {  \
    Expression exp(8); \
    exp.addFunction("add", [](QVariantList args) { return args.size() != 2 ? 0 : args[0].toInt() + args[1].toInt(); }); \
	ASSERT_EQ(b, exp.parse(a)) << a; \
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

	TEST_EXPR(TwoArgs, "add(22, 33)", 55)
	TEST_EXPR(Compound, "{3;3}", 3);
	TEST_EXPR(Compound1, "{3; 3}", 3)
	TEST_EXPR(Compound2, "{ 3;3 }", 3)

	TEST_EXPR(ArityError, "1+cos()", 0)
	TEST_EXPR(ArityError2, "add(10)", 0)
	TEST_EXPR(ArityError3, "add()", 0)



} // namespace

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}