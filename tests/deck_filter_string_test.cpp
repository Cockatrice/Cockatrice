#include "filters/deck_filter_string.h"

#include <gtest/gtest.h>
#include <libcockatrice/card/database/card_database.h>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/deck_list/tree/deck_list_card_node.h>
#include <limits>

namespace
{
class DeckFilterStringTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        auto *database = CardDatabaseManager::getInstance();
        database->clear();
        auto example = CardInfo::newInstance("Example");
        example->setProperty("cmc", "2");
        example->setLocalizedName("de", "Beispiel");
        database->addCard(example);
        deck.deckList.setName("My  Deck");
        deck.deckList.setComments("keep  both spaces");
        deck.deckList.addCard("Example", "main")->setNumber(1);
        data = {&deck, "/decks/My  Deck.cod", "My  Deck", "folder  name/My  Deck.cod"};
    }

    LoadedDeck deck;
    DeckSearchData data;
};
} // namespace

TEST_F(DeckFilterStringTest, PreservesWhitespaceInsideQuotedMetadata)
{
    for (const auto &query : {"name:\"My  Deck\"", "filename:\"My  Deck.cod\"", "path:\"folder  name\"",
                              "comment:\"keep  both spaces\"", "\"My  Deck\""}) {
        DeckFilterString filter(query);
        ASSERT_TRUE(filter.valid()) << filter.error().toStdString();
        EXPECT_TRUE(filter.check(data)) << query;
    }
    DeckFilterString differentName("name:\"My Deck\"");
    EXPECT_FALSE(differentName.check(data));
}

TEST_F(DeckFilterStringTest, AcceptsWhitespaceBetweenQueriesWithoutChangingQuotedValues)
{
    DeckFilterString filter(" \tname:\"My  Deck\"\tAND\ncomment:\"keep  both spaces\" \r\n");
    ASSERT_TRUE(filter.valid()) << filter.error().toStdString();
    EXPECT_TRUE(filter.check(data));
    EXPECT_TRUE(DeckFilterString(" \t\r\n").check(data));
}

TEST_F(DeckFilterStringTest, RejectsInvalidNestedQueriesEvenUnderNotOrOr)
{
    for (const auto &query :
         {"[[cmc>]]", "NOT [[cmc>]]", "-[[cmc>]]", "name:My OR [[cmc>]]", "NOT (name:Other OR [[cmc>]])"}) {
        DeckFilterString filter(query);
        EXPECT_FALSE(filter.valid()) << query;
        EXPECT_FALSE(filter.error().isEmpty()) << query;
        EXPECT_FALSE(filter.check(data)) << query;
    }
}

TEST_F(DeckFilterStringTest, CountsCopiesAndThresholdsBeyondThirtyTwoBits)
{
    deck.deckList.addCard("Example", "side")->setNumber(std::numeric_limits<int>::max());
    DeckFilterString exact("[[Example]]=2147483648");
    ASSERT_TRUE(exact.valid()) << exact.error().toStdString();
    EXPECT_TRUE(exact.check(data));
    EXPECT_TRUE(DeckFilterString("[[Example]]>2147483647").check(data));
    EXPECT_FALSE(DeckFilterString("[[Example]]>2147483648").check(data));
    EXPECT_TRUE(DeckFilterString("[[Example]]<9223372036854775807").check(data));
}

TEST_F(DeckFilterStringTest, LargeValidThresholdDoesNotBecomeZero)
{
    DeckFilterString filter("[[Example]]>2147483648");
    ASSERT_TRUE(filter.valid()) << filter.error().toStdString();
    EXPECT_FALSE(filter.check(data));
}

TEST_F(DeckFilterStringTest, RejectsOutOfRangeThresholds)
{
    for (const auto &query : {"[[Example]]>9223372036854775808", "[[Example]]=99999999999999999999",
                              "NOT [[Example]]>99999999999999999999"}) {
        DeckFilterString filter(query);
        EXPECT_FALSE(filter.valid()) << query;
        EXPECT_FALSE(filter.error().isEmpty()) << query;
        EXPECT_FALSE(filter.check(data)) << query;
    }
}

TEST_F(DeckFilterStringTest, SupportsOnlyDocumentedCountComparisons)
{
    for (const auto &query : {"[[Example]]=1", "[[Example]]:1", "[[Example]]>=1", "[[Example]]<=1", "[[Example]]!=0",
                              "[[Example]]>0", "[[Example]]<2"}) {
        DeckFilterString filter(query);
        EXPECT_TRUE(filter.valid()) << query;
        EXPECT_TRUE(filter.check(data)) << query;
    }
    DeckFilterString unsupported("[[Example]]!0");
    EXPECT_FALSE(unsupported.valid());
    EXPECT_FALSE(unsupported.check(data));
}

TEST_F(DeckFilterStringTest, NonpositiveEntriesDoNotSubtractMatchingCopies)
{
    deck.deckList.addCard("Example", "side")->setNumber(-10);
    EXPECT_TRUE(DeckFilterString("[[Example]]=1").check(data));
}

TEST_F(DeckFilterStringTest, ParseErrorsDoNotLeakIntoLaterQueries)
{
    DeckFilterString invalid("NOT [[cmc>]]");
    EXPECT_FALSE(invalid.valid());
    DeckFilterString valid("[[Example]]=1");
    EXPECT_TRUE(valid.valid()) << valid.error().toStdString();
    EXPECT_TRUE(valid.check(data));
    EXPECT_FALSE(invalid.check(data));
}

TEST_F(DeckFilterStringTest, NestedQueriesRetainTheirChosenSearchLanguage)
{
    DeckFilterString selected("[[Beispiel]]=1", {"de", SearchLanguageMode::Selected});
    DeckFilterString english("[[Beispiel]]=1", {"de", SearchLanguageMode::English});
    ASSERT_TRUE(selected.valid()) << selected.error().toStdString();
    ASSERT_TRUE(english.valid()) << english.error().toStdString();
    EXPECT_TRUE(selected.check(data));
    EXPECT_FALSE(english.check(data));
    EXPECT_TRUE(DeckFilterString("NOT [[cmc>2]] AND [[Example]]=1").check(data));
}
