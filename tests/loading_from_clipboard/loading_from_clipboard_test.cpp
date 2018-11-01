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

struct Result
{
    std::string name;
    std::string comments;
    CardRows mainboard;
    CardRows sideboard;

    Result(std::string _name, std::string _comments, CardRows _mainboard, CardRows _sideboard)
        : name(_name), comments(_comments), mainboard(_mainboard), sideboard(_sideboard)
    {
    }
};

void testEmpty(const QString &clipboard)
{
    QString cp(clipboard);
    DeckList deckList;
    QTextStream stream(&cp); // text stream requires local copy
    deckList.loadFromStream_Plain(stream);

    ASSERT_TRUE(deckList.getCardList().isEmpty());
}

void testDeck(const QString &clipboard, const Result &result)
{
    QString cp(clipboard);
    DeckList deckList;
    QTextStream stream(&cp); // text stream requires local copy
    deckList.loadFromStream_Plain(stream);

    ASSERT_EQ(result.name, deckList.getName().toStdString());
    ASSERT_EQ(result.comments, deckList.getComments().toStdString());

    DecklistBuilder decklistBuilder = DecklistBuilder();
    deckList.forEachCard(decklistBuilder);

    ASSERT_EQ(result.mainboard, decklistBuilder.mainboard());
    ASSERT_EQ(result.sideboard, decklistBuilder.sideboard());
}

namespace
{
TEST(LoadingFromClipboardTest, EmptyDeck)
{
    testEmpty("");
}

TEST(LoadingFromClipboardTest, EmptySideboard)
{
    testEmpty("Sideboard");
}

TEST(LoadingFromClipboardTest, QuantityPrefixed)
{
    QString clipboard("1 Mountain\n"
                      "2x Island\n"
                      "3X FOREST\n");
    Result result("", "", {{"mountain", 1}, {"island", 2}, {"forest", 3}}, {});
    testDeck(clipboard, result);
}

TEST(LoadingFromClipboardTest, CommentsAreIgnored)
{
    QString clipboard("//1 Mountain\n"
                      "//2x Island\n"
                      "//SB:2x Island\n");
    testEmpty(clipboard);
}

TEST(LoadingFromClipboardTest, SideboardPrefix)
{
    QString clipboard("1 Mountain\n"
                      "SB: 1 Mountain\n"
                      "SB: 2x Island\n"
                      "2 Swamp\n"
                      "\n"
                      "3 plains\n");
    Result result("", "", {{"mountain", 1}, {"swamp", 2}, {"plains", 3}}, {{"mountain", 1}, {"island", 2}});
    testDeck(clipboard, result);
}

TEST(LoadingFromClipboardTest, SideboardLine)
{
    QString clipboard("1 Mountain\n"
                      "2 Swamp\n"
                      "\n"
                      "3 plains\n"
                      "sideboard\n"
                      "1 Mountain\n"
                      "2x Island\n");
    Result result("", "", {{"mountain", 1}, {"swamp", 2}, {"plains", 3}}, {{"mountain", 1}, {"island", 2}});
    testDeck(clipboard, result);
}

TEST(LoadingFromClipboardTest, UnknownCardsAreNotDiscarded)
{
    QString clipboard("1 CardThatDoesNotExistInCardsXml\n");
    Result result("", "", {{"cardthatdoesnotexistincardsxml", 1}}, {});
    testDeck(clipboard, result);
}

TEST(LoadingFromClipboardTest, WeirdWhitespaceIsIgnored)
{
    QString clipboard("  Our Market   Research Shows That Players Like Really Long Card Names           So We Made "
                      "this Card to Have\tthe Absolute \t Longest Card Name \tEver Elemental\t\n\t");
    Result result("", "",
                  {{"our market research shows that players like really long card names so we made this card to have "
                    "the absolute longest card name ever elemental",
                    1}},
                  {});
    testDeck(clipboard, result);
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

    Result result("", "", {{"algae gharial", 1}, {"cardthatdoesnotexistincardsxml", 3}, {"phelddagrif", 2}}, {});
    testDeck(clipboard, result);
}

TEST(LoadingFromClipboardTest, UseFirstBlankIfOnlyOneBlankToSplitSideboard)
{
    QString clipboard("1x Algae Gharial\n"
                      "3x CardThatDoesNotExistInCardsXml\n"
                      "\n"
                      "2x Phelddagrif\n");

    Result result("", "", {{"algae gharial", 1}, {"cardthatdoesnotexistincardsxml", 3}}, {{"phelddagrif", 2}});
    testDeck(clipboard, result);
}

TEST(LoadingFromClipboardTest, IfMultipleScatteredBlanksAllMainBoard)
{
    QString clipboard("1x Algae Gharial\n"
                      "3x CardThatDoesNotExistInCardsXml\n"
                      "\n"
                      "2x Phelddagrif\n"
                      "\n"
                      "3 Giant Growth\n");

    Result result(
        "", "", {{"algae gharial", 1}, {"cardthatdoesnotexistincardsxml", 3}, {"phelddagrif", 2}, {"giant growth", 3}},
        {});
    testDeck(clipboard, result);
}

TEST(LoadingFromClipboardTest, LotsOfStuffInBulkTesting)
{
    QString clipboard("\n"
                      "\n"
                      "\n"
                      "1x Æther Adept\n"
                      "2x Fire & Ice\n"
                      "3 Pain/Suffering\n"
                      "4X [B] FOREST (3)\n"
                      "testNoValueMB\n"
                      "\n"
                      "\n"
                      "// I like cards\n"
                      "\n"
                      "\n"
                      "5x [WTH] nature’s resurgence\n"
                      "6X Gaea's skYFOLK\n"
                      "7  B.F.M. (Big Furry Monster)\n"
                      "testNoValueSB\n"
                      "\n"
                      "\n"
                      "\n"
                      "\n");

    Result result(
        "", "",
        {{"aether adept", 1}, {"fire // ice", 2}, {"pain // suffering", 3}, {"forest", 4}, {"testnovaluemb", 1}},
        {{"nature's resurgence", 5}, {"gaea's skyfolk", 6}, {"b.f.m. (big furry monster)", 7}, {"testnovaluesb", 1}});
    testDeck(clipboard, result);
}

TEST(LoadingFromClipboardTest, CommentsBeforeCardsTesting)
{
    QString clipboard("// Title from website.com\n"
                      "// A nice deck\n"
                      "// With nice cards\n"
                      "\n"
                      "// Mainboard\n"
                      "1 test1\n"
                      "Sideboard\n"
                      "2 test2\n");

    Result result("Title from website.com", "A nice deck\nWith nice cards", {{"test1", 1}}, {{"test2", 2}});
    testDeck(clipboard, result);
}
} // namespace

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
