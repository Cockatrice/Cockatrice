#include "libcockatrice/utility/svg_recolor.h"

#include <QByteArray>
#include <gtest/gtest.h>

namespace
{

TEST(SvgRecolor, ReplacesEverySentinel)
{
    const QByteArray svg = R"(<svg><path fill="currentColor"/><path stroke="currentColor"/></svg>)";
    const QByteArray result = resolveSvgColorSentinel(svg, "#ff8800");

    EXPECT_EQ(result.count("#ff8800"), 2);
    EXPECT_FALSE(result.contains("currentColor"));
}

TEST(SvgRecolor, ReplacesSentinelRegardlessOfCase)
{
    const QByteArray svg = R"(<svg><path fill="CurrentColor"/><path stroke="CURRENTCOLOR"/></svg>)";

    EXPECT_EQ(resolveSvgColorSentinel(svg, "#ff8800").count("#ff8800"), 2);
}

TEST(SvgRecolor, LeavesTheRestOfTheDocumentIntact)
{
    const QByteArray svg =
        R"(<svg viewBox="0 0 24 24"><!-- an ordinary comment --><path d="M0 0h24v24" fill="currentColor"/></svg>)";
    const QByteArray result = resolveSvgColorSentinel(svg, "#123456");

    EXPECT_TRUE(result.contains(R"(viewBox="0 0 24 24")"));
    EXPECT_TRUE(result.contains(R"(d="M0 0h24v24")"));
    EXPECT_TRUE(result.contains("an ordinary comment"));
}

TEST(SvgRecolor, DoesNotModifyTheInput)
{
    const QByteArray svg = R"(<svg><path fill="currentColor"/></svg>)";
    const QByteArray original = svg;

    resolveSvgColorSentinel(svg, "#000000");

    EXPECT_EQ(svg, original);
}

TEST(SvgRecolor, PreservesBytesThatAreNotValidUtf8)
{
    QByteArray svg = "<svg>";
    svg.append("\xff\xfe");
    svg.append(R"(<path fill="currentColor"/></svg>)");

    const QByteArray result = resolveSvgColorSentinel(svg, "#ff8800");

    EXPECT_TRUE(result.contains("\xff\xfe"));
    EXPECT_FALSE(result.contains("currentColor"));
    EXPECT_EQ(result.count("#ff8800"), 1);
}

TEST(SvgRecolor, NoSentinelIsANoOp)
{
    const QByteArray svg = R"(<svg><path fill="#abcdef"/></svg>)";

    EXPECT_EQ(resolveSvgColorSentinel(svg, "#ffffff"), svg);
}

} // namespace
