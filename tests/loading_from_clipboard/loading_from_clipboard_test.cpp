#include "../../common/decklist.h"
#include "gtest/gtest.h"
#include <QTextStream>

// using std types because qt types aren't understood by gtest (without this you'll get less nice errors)
using CardRows = QList<std::pair<std::string, int>>;

struct DecklistBuilder
{
    CardRows actualMainboard;
    CardRows actualSideboard;

    explicit DecklistBuilder() : actualMainboard({}), actualSideboard({})
    {
    }

    void operator()(const InnerDecklistNode *innerDecklistNode, const DecklistCardNode *card)
    {
        if (innerDecklistNode->getName() == DECK_ZONE_MAIN) {
            actualMainboard += {card->getName().toStdString(), card->getNumber()};
        } else if (innerDecklistNode->getName() == DECK_ZONE_SIDE) {
            actualSideboard += {card->getName().toStdString(), card->getNumber()};
        } else {
            FAIL();
        }
    }

    CardRows mainboard()
    {
        return actualMainboard;
    }

    CardRows sideboard()
    {
        return actualSideboard;
    }
};

namespace
{
TEST(LoadingFromClipboardTest, EmptyDeck)
{
    DeckList deckList;
    QString clipboard("");
    QTextStream stream(&clipboard);
    deckList.loadFromStream_Plain(stream);
    ASSERT_TRUE(deckList.getCardList().isEmpty());
}

TEST(LoadingFromClipboardTest, EmptySideboard)
{
    DeckList deckList;
    QString clipboard("Sideboard");
    QTextStream stream(&clipboard);
    deckList.loadFromStream_Plain(stream);
    ASSERT_TRUE(deckList.getCardList().isEmpty());
}

TEST(LoadingFromClipboardTest, QuantityPrefixed)
{
    QString clipboard("1 Mountain\n"
                      "2x Island\n"
                      "3X FOREST\n");
    DeckList deckList;
    QTextStream stream(&clipboard);
    deckList.loadFromStream_Plain(stream);

    DecklistBuilder decklistBuilder = DecklistBuilder();
    deckList.forEachCard(decklistBuilder);

    CardRows expectedMainboard = CardRows({{"mountain", 1}, {"island", 2}, {"forest", 3}});
    CardRows expectedSideboard = CardRows({});

    ASSERT_EQ(expectedMainboard, decklistBuilder.mainboard());
    ASSERT_EQ(expectedSideboard, decklistBuilder.sideboard());
}

TEST(LoadingFromClipboardTest, CommentsAreIgnored)
{
    QString clipboard("//1 Mountain\n"
                      "//2x Island\n"
                      "//SB:2x Island\n");

    DeckList deckList;
    QTextStream stream(&clipboard);
    deckList.loadFromStream_Plain(stream);

    DecklistBuilder decklistBuilder = DecklistBuilder();
    deckList.forEachCard(decklistBuilder);

    CardRows expectedMainboard = CardRows({});
    CardRows expectedSideboard = CardRows({});

    ASSERT_EQ(expectedMainboard, decklistBuilder.mainboard());
    ASSERT_EQ(expectedSideboard, decklistBuilder.sideboard());
}

TEST(LoadingFromClipboardTest, SideboardPrefix)
{
    QString clipboard("1 Mountain\n"
                      "SB: 1 Mountain\n"
                      "SB: 2x Island\n");
    DeckList deckList;
    QTextStream stream(&clipboard);
    deckList.loadFromStream_Plain(stream);

    DecklistBuilder decklistBuilder = DecklistBuilder();
    deckList.forEachCard(decklistBuilder);

    CardRows expectedMainboard = CardRows({{"mountain", 1}});
    CardRows expectedSideboard = CardRows({{"mountain", 1}, {"island", 2}});

    ASSERT_EQ(expectedMainboard, decklistBuilder.mainboard());
    ASSERT_EQ(expectedSideboard, decklistBuilder.sideboard());
}

TEST(LoadingFromClipboardTest, UnknownCardsAreNotDiscarded)
{
    QString clipboard("1 CardThatDoesNotExistInCardsXml\n");
    DeckList deckList;
    QTextStream stream(&clipboard);
    deckList.loadFromStream_Plain(stream);

    DecklistBuilder decklistBuilder = DecklistBuilder();
    deckList.forEachCard(decklistBuilder);

    CardRows expectedMainboard = CardRows({{"cardthatdoesnotexistincardsxml", 1}});
    CardRows expectedSideboard = CardRows({});

    ASSERT_EQ(expectedMainboard, decklistBuilder.mainboard());
    ASSERT_EQ(expectedSideboard, decklistBuilder.sideboard());
}

TEST(LoadingFromClipboardTest, RemoveBlankEntriesFromBeginningAndEnd)
{
    QString clipboard("\n"
                      "\n"
                      "\n"
                      "1x Algae Gharial\n"
                      "3x CardThatDoesNotExistInCardsXml\n"
                      "2x Phelddagrif\n"
                      "\n"
                      "\n");

    DeckList deckList;
    QTextStream stream(&clipboard);
    deckList.loadFromStream_Plain(stream);

    DecklistBuilder decklistBuilder = DecklistBuilder();
    deckList.forEachCard(decklistBuilder);

    CardRows expectedMainboard =
        CardRows({{"algae gharial", 1}, {"cardthatdoesnotexistincardsxml", 3}, {"phelddagrif", 2}});
    CardRows expectedSideboard = CardRows({});

    ASSERT_EQ(expectedMainboard, decklistBuilder.mainboard());
    ASSERT_EQ(expectedSideboard, decklistBuilder.sideboard());
}

TEST(LoadingFromClipboardTest, UseFirstBlankIfOnlyOneBlankToSplitSideboard)
{
    QString clipboard("1x Algae Gharial\n"
                      "3x CardThatDoesNotExistInCardsXml\n"
                      "\n"
                      "2x Phelddagrif\n");

    DeckList deckList;
    QTextStream stream(&clipboard);
    deckList.loadFromStream_Plain(stream);

    DecklistBuilder decklistBuilder = DecklistBuilder();
    deckList.forEachCard(decklistBuilder);

    CardRows expectedMainboard = CardRows({{"algae gharial", 1}, {"cardthatdoesnotexistincardsxml", 3}});
    CardRows expectedSideboard = CardRows({{"phelddagrif", 2}});

    ASSERT_EQ(expectedMainboard, decklistBuilder.mainboard());
    ASSERT_EQ(expectedSideboard, decklistBuilder.sideboard());
}

TEST(LoadingFromClipboardTest, IfMultipleScatteredBlanksAllMainBoard)
{
    QString clipboard("1x Algae Gharial\n"
                      "3x CardThatDoesNotExistInCardsXml\n"
                      "\n"
                      "2x Phelddagrif\n"
                      "\n"
                      "3 Giant Growth\n");

    DeckList deckList;
    QTextStream stream(&clipboard);
    deckList.loadFromStream_Plain(stream);

    DecklistBuilder decklistBuilder = DecklistBuilder();
    deckList.forEachCard(decklistBuilder);

    CardRows expectedMainboard = CardRows(
        {{"algae gharial", 1}, {"cardthatdoesnotexistincardsxml", 3}, {"phelddagrif", 2}, {"giant growth", 3}});
    CardRows expectedSideboard = CardRows({});

    ASSERT_EQ(expectedMainboard, decklistBuilder.mainboard());
    ASSERT_EQ(expectedSideboard, decklistBuilder.sideboard());
}

TEST(LoadingFromClipboardTest, LotsOfStuffInBulkTesting)
{
    QString clipboard("\n"
                      "\n"
                      "\n"
                      "1x aether adept\n" // "1x Æther Adept\n" // bugged
                      "2x Fire & Ice\n"
                      "3 Pain/Suffering\n"
                      "4X [B] FOREST (3)\n"
                      "testNoValueMB\n"
                      "\n"
                      "\n"
                      "\n"
                      "\n"
                      "5x [WTH] nature’s resurgence\n"
                      "6X Gaea's skYFOLK\n"
                      "testNoValueSB\n"
                      "\n"
                      "\n"
                      "\n"
                      "\n");

    DeckList deckList;
    QTextStream stream(&clipboard);
    deckList.loadFromStream_Plain(stream);

    DecklistBuilder decklistBuilder = DecklistBuilder();
    deckList.forEachCard(decklistBuilder);

    CardRows expectedMainboard = CardRows(
        {{"aether adept", 1}, {"fire // ice", 2}, {"pain // suffering", 3}, {"forest", 4}, {"testnovaluemb", 1}});
    CardRows expectedSideboard = CardRows({{"nature's resurgence", 5}, {"gaea's skyfolk", 6}, {"testnovaluesb", 1}});
    ASSERT_EQ(expectedMainboard, decklistBuilder.mainboard());
    ASSERT_EQ(expectedSideboard, decklistBuilder.sideboard());
}

TEST(LoadingFromClipboardTest, CommentsBeforeCardsTesting)
{
    QString clipboard("// title from website.com\n"
                      "// a nice deck\n"
                      "// with nice cards\n"
                      "\n"
                      "1 test1\n"
                      "sideboard\n"
                      "2 test2\n");

    DeckList deckList;
    QTextStream stream(&clipboard);
    deckList.loadFromStream_Plain(stream);

    ASSERT_EQ(deckList.getName().toStdString(), "title from website.com");
    ASSERT_EQ(deckList.getComments().toStdString(), "a nice deck\nwith nice cards\n");

    DecklistBuilder decklistBuilder = DecklistBuilder();
    deckList.forEachCard(decklistBuilder);

    CardRows expectedMainboard = CardRows({{"test1", 1}});
    CardRows expectedSideboard = CardRows({{"test2", 2}});

    ASSERT_EQ(expectedMainboard, decklistBuilder.mainboard());
    ASSERT_EQ(expectedSideboard, decklistBuilder.sideboard());
}
} // namespace

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
