/** @file deck_filter_string_test.cpp
 *  @brief Tests for DeckFilterString's parse-error reporting.
 *  @ingroup Tests
 */

#include <deck_filter_string.h>
#include <gtest/gtest.h>

namespace
{
/// The parser is a file-static permanent object whose logger is reinstalled by every
/// constructor, so these tests pin the observable contract that no state leaks from one
/// instance into the next, and that a destroyed instance cannot disturb a later parse.
QString parseErrorFor(const QString &expression)
{
    DeckFilterString filter(expression);
    return filter.valid() ? QString() : filter.error();
}
} // namespace

TEST(DeckFilterStringError, ValidExpressionHasNoError)
{
    EXPECT_TRUE(parseErrorFor("name:foo").isEmpty());
    EXPECT_TRUE(parseErrorFor("[[Lightning Bolt]]").isEmpty());
}

TEST(DeckFilterStringError, InvalidExpressionReportsAPosition)
{
    const QString error = parseErrorFor("name:foo AND (");
    ASSERT_FALSE(error.isEmpty());
    EXPECT_TRUE(error.contains("Error at position")) << qPrintable(error);
}

TEST(DeckFilterStringError, DefaultConstructedIsNotValid)
{
    DeckFilterString filter;
    EXPECT_FALSE(filter.valid());
    EXPECT_EQ(filter.error(), QString("Not initialized"));
}

TEST(DeckFilterStringError, EmptyExpressionIsValid)
{
    EXPECT_TRUE(parseErrorFor("").isEmpty());
    EXPECT_TRUE(parseErrorFor("   ").isEmpty());
}

// An error must not survive into an expression that parses. The parser is permanent and
// keeps its logger between calls, so a stale message here means state leaked.
TEST(DeckFilterStringError, ErrorDoesNotLeakIntoTheNextValidParse)
{
    ASSERT_FALSE(parseErrorFor("name:foo AND (").isEmpty());
    EXPECT_TRUE(parseErrorFor("name:foo").isEmpty());
}

TEST(DeckFilterStringError, ErrorDoesNotLeakIntoTheNextInvalidParse)
{
    const QString first = parseErrorFor("name:foo AND (");
    const QString second = parseErrorFor("format:");

    ASSERT_FALSE(first.isEmpty());
    ASSERT_FALSE(second.isEmpty());
    // Each parse reports its own position. If the second reused the first's message, the
    // message would still be non-empty but would name the wrong offset.
    EXPECT_TRUE(second.contains("position 8")) << qPrintable(second);
    EXPECT_TRUE(first.contains("position 15")) << qPrintable(first);
}

// Two filters can be live at once, so each must report only its own error.
TEST(DeckFilterStringError, SecondInstanceDoesNotDisturbTheFirst)
{
    DeckFilterString first("name:foo AND (");
    const QString firstError = first.error();
    ASSERT_FALSE(firstError.isEmpty());

    {
        DeckFilterString second("format:");
        ASSERT_FALSE(second.valid());
    }

    EXPECT_FALSE(first.valid());
    EXPECT_EQ(first.error(), firstError);
}

TEST(DeckFilterStringError, DestroyedInstanceDoesNotDisturbALaterParse)
{
    {
        DeckFilterString doomed("name:foo AND (");
        ASSERT_FALSE(doomed.valid());
    }

    // A parse after an instance has gone must still report its own error.
    const QString error = parseErrorFor("name:foo AND name:bar AND (");
    ASSERT_FALSE(error.isEmpty());
    EXPECT_TRUE(error.contains("position 28")) << qPrintable(error);
}

TEST(DeckFilterStringError, ManyInstancesInSequenceEachReportThemselves)
{
    // Long enough that an instance and its successor land on the same reused heap block,
    // so a stale read would show up rather than passing by luck.
    for (int i = 0; i < 200; ++i) {
        const QString invalid = parseErrorFor("name:foo AND (");
        ASSERT_FALSE(invalid.isEmpty()) << "iteration " << i;
        ASSERT_TRUE(parseErrorFor("name:foo").isEmpty()) << "iteration " << i;
    }
}
