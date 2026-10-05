/** @file server_dump_zone_test.cpp
 *  @brief Tests for resolving a client's zone dump request against a zone's card count.
 *  @ingroup Tests
 */

#include <gtest/gtest.h>
#include <climits>
#include <vector>
#include <libcockatrice/network/server/remote/game/dump_zone_range.h>
#include <QList>

namespace
{
/// Indices a dump request resolves to, as the handler would walk them.
std::vector<qsizetype> walked(qsizetype cardCount, int numberCards, bool isReversed)
{
    const auto [begin, end] = dumpZoneCardRange(cardCount, numberCards, isReversed);
    std::vector<qsizetype> out;
    for (qsizetype i = begin; i < end; ++i) {
        out.push_back(i);
    }
    return out;
}
} // namespace

TEST(ServerDumpZone, ForwardTakesLeadingCards)
{
    EXPECT_EQ(walked(5, 2, false), (std::vector<qsizetype>{ 0, 1 }));
}

TEST(ServerDumpZone, ForwardTakingAllCardsCoversTheZone)
{
    EXPECT_EQ(walked(3, 3, false), (std::vector<qsizetype>{ 0, 1, 2 }));
}

TEST(ServerDumpZone, ForwardAskingForOneDumpsOne)
{
    EXPECT_EQ(walked(3, 1, false), (std::vector<qsizetype>{ 0 }));
}

TEST(ServerDumpZone, MinusOneMeansTheWholeZone)
{
    EXPECT_EQ(walked(4, -1, false), (std::vector<qsizetype>{ 0, 1, 2, 3 }));
}

TEST(ServerDumpZone, ReversedTakesTrailingCards)
{
    EXPECT_EQ(walked(5, 2, true), (std::vector<qsizetype>{ 3, 4 }));
}

TEST(ServerDumpZone, ReversedAskingForOneTakesTheLast)
{
    EXPECT_EQ(walked(5, 1, true), (std::vector<qsizetype>{ 4 }));
}

TEST(ServerDumpZone, ZeroDumpsNothing)
{
    EXPECT_TRUE(walked(5, 0, false).empty());
    EXPECT_TRUE(walked(5, 0, true).empty());
}

// A request for more cards than the zone holds is clamped rather than walked off the end.
// Forward, the old loop stopped at the zone's size on its own.
TEST(ServerDumpZone, ForwardOversizedRequestIsClamped)
{
    EXPECT_EQ(walked(3, 1000, false), (std::vector<qsizetype>{ 0, 1, 2 }));
}

// Reversed, an oversized request put the offset at cardCount - numberCards, which is
// negative, so the walk started before the first card.
TEST(ServerDumpZone, ReversedOversizedRequestIsClamped)
{
    EXPECT_EQ(walked(3, 1000, true), (std::vector<qsizetype>{ 0, 1, 2 }));
}

// Reversed, -1 put the offset at cardCount + 1, so the walk started one past the last card.
TEST(ServerDumpZone, ReversedWholeZoneRequestStaysInRange)
{
    EXPECT_EQ(walked(3, -1, true), (std::vector<qsizetype>{ 0, 1, 2 }));
}

// Only -1 means "the whole zone". Any other negative count is not a request for anything,
// so it resolves to an empty range rather than to the whole zone.
TEST(ServerDumpZone, OtherNegativeCountsDumpNothing)
{
    EXPECT_TRUE(walked(5, -2, false).empty());
    EXPECT_TRUE(walked(5, -2, true).empty());
    EXPECT_TRUE(walked(5, INT_MIN, true).empty());
}

TEST(ServerDumpZone, EmptyZoneYieldsAnEmptyRange)
{
    EXPECT_TRUE(walked(0, -1, false).empty());
    EXPECT_TRUE(walked(0, -1, true).empty());
    EXPECT_TRUE(walked(0, 10, false).empty());
    EXPECT_TRUE(walked(0, 10, true).empty());
}

TEST(ServerDumpZone, NonPositiveCardCountYieldsAnEmptyRange)
{
    EXPECT_TRUE(walked(-1, -1, false).empty());
    EXPECT_TRUE(walked(-1, -1, true).empty());
}

// The invariant the handler depends on: whatever a client asks for, the indices it walks
// are a subset of the zone's cards and never step outside the list. number_cards is an
// unbounded client-supplied sint32, so this sweeps the edges rather than a chosen few.
TEST(ServerDumpZone, ResolvedRangeIsAlwaysWithinTheZone)
{
    const QList<int> cardCounts = { 0, 1, 2, 3, 7, 60, 1000000 };
    const QList<int> requests = { INT_MIN,     -1000,     -2, -1, 0, 1, 2, 3,
                                  59,         60,        61, 999999, INT_MAX };

    for (int cardCount : cardCounts) {
        for (int request : requests) {
            for (bool isReversed : { false, true }) {
                const auto [begin, end] = dumpZoneCardRange(cardCount, request, isReversed);
                SCOPED_TRACE(testing::Message() << "cardCount=" << cardCount << " request=" << request
                                                << " isReversed=" << isReversed);
                ASSERT_GE(begin, 0);
                ASSERT_GE(end, begin);
                ASSERT_LE(end, cardCount);
            }
        }
    }
}
