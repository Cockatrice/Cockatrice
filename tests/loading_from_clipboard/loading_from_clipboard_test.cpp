#include "clipboard_testing.h"

// Testing is done by using the DeckList::loadFromString_Plain function in common/decklist.h
// It does not check if cards are in the database at all, so no comparisons to the database will be made.

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
                      "3x Forest\n");
    Result result("", "", {{"Mountain", 1}, {"Island", 2}, {"Forest", 3}}, {});
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
                      "sb: 2x Island\n"
                      "2 Swamp\n"
                      "\n"
                      "3 Plains\n");
    Result result("", "", {{"Mountain", 1}, {"Swamp", 2}, {"Plains", 3}}, {{"Mountain", 1}, {"Island", 2}});
    testDeck(clipboard, result);
}

TEST(LoadingFromClipboardTest, SideboardLine)
{
    QString clipboard("1 Mountain\n"
                      "2 Swamp\n"
                      "\n"
                      "3 Plains\n"
                      "sideboard\n"
                      "1 Mountain\n"
                      "2x Island\n");
    Result result("", "", {{"Mountain", 1}, {"Swamp", 2}, {"Plains", 3}}, {{"Mountain", 1}, {"Island", 2}});
    testDeck(clipboard, result);
}

TEST(LoadingFromClipboardTest, UnknownCardsAreNotDiscarded)
{
    QString clipboard("1 CardThatDoesNotExistInCardsXml\n");
    Result result("", "", {{"CardThatDoesNotExistInCardsXml", 1}}, {});
    testDeck(clipboard, result);
}

TEST(LoadingFromClipboardTest, WeirdWhitespaceIsIgnored)
{
    QString clipboard(
        "\t\tSb:\t1\tOur Market   Research Shows That Players Like  Really Long Card Names           So We Made        "
        "           This Card to Have\tthe Absolute \t Longest Card Name \tEver Elemental\t\n\t");
    Result result("", "", {},
                  {{"Our Market Research Shows That Players Like Really Long Card Names So We Made This Card to Have "
                    "the Absolute Longest Card Name Ever Elemental",
                    1}});
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

    Result result("", "", {{"Algae Gharial", 1}, {"CardThatDoesNotExistInCardsXml", 3}, {"Phelddagrif", 2}}, {});
    testDeck(clipboard, result);
}

TEST(LoadingFromClipboardTest, UseFirstBlankIfOnlyOneBlankToSplitSideboard)
{
    QString clipboard("1x Algae Gharial\n"
                      "3x CardThatDoesNotExistInCardsXml\n"
                      "\n"
                      "2x Phelddagrif\n");

    Result result("", "", {{"Algae Gharial", 1}, {"CardThatDoesNotExistInCardsXml", 3}}, {{"Phelddagrif", 2}});
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
        "", "", {{"Algae Gharial", 1}, {"CardThatDoesNotExistInCardsXml", 3}, {"Phelddagrif", 2}, {"Giant Growth", 3}},
        {});
    testDeck(clipboard, result);
}

TEST(LoadingFromClipboardTest, EdgeCaseTesting)
{
    QString clipboard(R"(
// DeckName

   // Comment 1

//
//Comment [two]
//(test) Æ ’ | / (3)


// Mainboard (11 cards)
Æther Adept
2x Fire // Ice
1 Minsc & Boo, Timeless Heroes
3 Pain/Suffering
4X [B] Forest (3)


// Sideboard (11 cards)

5x [WTH] Nature’s Resurgence
6X Gaea's Skyfolk
7  B.F.M. (Big Furry Monster)



)");

    Result result("DeckName", "Comment 1\n\nComment [two]\n(test) Æ ’ | / (3)",
                  {{"Aether Adept", 1},
                   {"Fire // Ice", 2},
                   {"Minsc & Boo, Timeless Heroes", 1},
                   {"Pain // Suffering", 3},
                   {"Forest", 4}},
                  {{"Nature's Resurgence", 5}, {"Gaea's Skyfolk", 6}, {"B.F.M. (Big Furry Monster)", 7}});
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

TEST(LoadingFromClipboardTest, mainboardAsLine)
{
    QString clipboard("// Deck Name\n"
                      "\n"
                      "MainBoard: 3 cards\n"
                      "3 card\n"
                      "\n"
                      "SideBoard: 2 cards\n"
                      "2 sidecard\n");

    Result result("Deck Name", "", {{"card", 3}}, {{"sidecard", 2}});
    testDeck(clipboard, result);
}

TEST(LoadingFromClipboardTest, deckAsCard)
{
    QString clipboard("6 Deck of Cards But Animated\n"
                      "\n"
                      "7 Sideboard Card\n");

    Result result("", "", {{"Deck of Cards But Animated", 6}}, {{"Sideboard Card", 7}});
    testDeck(clipboard, result);
}

TEST(LoadingFromClipboardTest, emptyMainBoard)
{
    QString clipboard("deck\n"
                      "\n"
                      "sideboard\n");

    testEmpty(clipboard);
}

TEST(LoadingFromClipboardTest, emptyHash)
{
    QString clipboard("");

    testHash(clipboard, "r8sq7riu");
}

TEST(LoadingFromClipboardTest, deckHash)
{
    QString clipboard("1 Mountain\n"
                      "2 Island\n"
                      "SB: 3 Forest\n");

    testHash(clipboard, "5cac19qm");
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
