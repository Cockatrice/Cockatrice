#include "gtest/gtest.h"
#include "loading_from_clipboard_test.h"
#include <QTextStream>
#include "../../common/decklist.h"

DeckList *fromClipboard(QString *clipboard);
DeckList *fromClipboard(QString *clipboard) {
    DeckList *deckList = new DeckList;
    QTextStream *stream = new QTextStream(clipboard);
    deckList->loadFromStream_Plain(*stream);
    return deckList;
}

using CardRows = QMap<QString, int>;

struct DecklistBuilder {
    CardRows actualMainboard;
    CardRows actualSideboard;

    explicit DecklistBuilder() : actualMainboard({}), actualSideboard({}) {}

    void operator()(const InnerDecklistNode *innerDecklistNode, const DecklistCardNode *card) {
        if (innerDecklistNode->getName() == DECK_ZONE_MAIN) {
            actualMainboard[card->getName()] += card->getNumber();
        } else if (innerDecklistNode->getName() == DECK_ZONE_SIDE) {
            actualSideboard[card->getName()] += card->getNumber();
        } else {
            FAIL();
        }
    }

    CardRows mainboard() {
        return actualMainboard;
    }

    CardRows sideboard() {
        return actualSideboard;
    }
};

namespace {
    TEST(LoadingFromClipboardTest, EmptyDeck)
    {
        DeckList *deckList = fromClipboard(new QString(""));
        ASSERT_TRUE(deckList->getCardList().isEmpty());
    }

    TEST(LoadingFromClipboardTest, EmptySideboard) {
        DeckList *deckList = fromClipboard(new QString("Sideboard"));
        ASSERT_TRUE(deckList->getCardList().isEmpty());
    }

    TEST(LoadingFromClipboardTest, QuantityPrefixed) {
        QString *clipboard = new QString(
                "1 Mountain\n"
                        "2x Island\n"
                        "3X FOREST\n"
        );
        DeckList *deckList = fromClipboard(clipboard);

        DecklistBuilder decklistBuilder = DecklistBuilder();
        deckList->forEachCard(decklistBuilder);

        CardRows expectedMainboard = CardRows({
                                                      {"mountain", 1},
                                                      {"island", 2},
                                                      {"forest", 3}
                                              });
        CardRows expectedSideboard = CardRows({});

        ASSERT_EQ(expectedMainboard, decklistBuilder.mainboard());
        ASSERT_EQ(expectedSideboard, decklistBuilder.sideboard());
    }

    TEST(LoadingFromClipboardTest, CommentsAreIgnored) {
        QString *clipboard = new QString(
                "//1 Mountain\n"
                        "//2x Island\n"
                        "//SB:2x Island\n"
        );

        DeckList *deckList = fromClipboard(clipboard);

        DecklistBuilder decklistBuilder = DecklistBuilder();
        deckList->forEachCard(decklistBuilder);

        CardRows expectedMainboard = CardRows({});
        CardRows expectedSideboard = CardRows({});

        ASSERT_EQ(expectedMainboard, decklistBuilder.mainboard());
        ASSERT_EQ(expectedSideboard, decklistBuilder.sideboard());
    }

    TEST(LoadingFromClipboardTest, SideboardPrefix) {
        QString *clipboard = new QString(
                "1 Mountain\n"
                        "SB: 1 Mountain\n"
                        "SB: 2x Island\n"
        );
        DeckList *deckList = fromClipboard(clipboard);

        DecklistBuilder decklistBuilder = DecklistBuilder();
        deckList->forEachCard(decklistBuilder);

        CardRows expectedMainboard = CardRows({
                                                      {"mountain", 1}
                                              });
        CardRows expectedSideboard = CardRows({
                                                      {"mountain", 1},
                                                      {"island",   2}
                                              });

        ASSERT_EQ(expectedMainboard, decklistBuilder.mainboard());
        ASSERT_EQ(expectedSideboard, decklistBuilder.sideboard());
    }

    TEST(LoadingFromClipboardTest, UnknownCardsAreNotDiscarded) {
        QString *clipboard = new QString(
                "1 CardThatDoesNotExistInCardsXml\n"
        );
        DeckList *deckList = fromClipboard(clipboard);

        DecklistBuilder decklistBuilder = DecklistBuilder();
        deckList->forEachCard(decklistBuilder);

        CardRows expectedMainboard = CardRows({
                                                      {"cardthatdoesnotexistincardsxml", 1}
                                              });
        CardRows expectedSideboard = CardRows({});

        ASSERT_EQ(expectedMainboard, decklistBuilder.mainboard());
        ASSERT_EQ(expectedSideboard, decklistBuilder.sideboard());
    }

    TEST(LoadingFromClipboardTest, RemoveBlankEntriesFromBeginningAndEnd) {
        QString *clipboard = new QString(
                "\n"
                        "\n"
                        "\n"
                        "1x Algae Gharial\n"
                        "3x CardThatDoesNotExistInCardsXml\n"
                        "2x Phelddagrif\n"
                        "\n"
                        "\n"
        );

        DeckList *deckList = fromClipboard(clipboard);

        DecklistBuilder decklistBuilder = DecklistBuilder();
        deckList->forEachCard(decklistBuilder);

        CardRows expectedMainboard = CardRows({
                                                      {"algae gharial", 1},
                                                      {"cardthatdoesnotexistincardsxml", 3},
                                                      {"phelddagrif", 2}
                                              });
        CardRows expectedSideboard = CardRows({});

        ASSERT_EQ(expectedMainboard, decklistBuilder.mainboard());
        ASSERT_EQ(expectedSideboard, decklistBuilder.sideboard());
    }

    TEST(LoadingFromClipboardTest, UseFirstBlankIfOnlyOneBlankToSplitSideboard) {
        QString *clipboard = new QString(
                "1x Algae Gharial\n"
                        "3x CardThatDoesNotExistInCardsXml\n"
                        "\n"
                        "2x Phelddagrif\n"
        );

        DeckList *deckList = fromClipboard(clipboard);

        DecklistBuilder decklistBuilder = DecklistBuilder();
        deckList->forEachCard(decklistBuilder);

        CardRows expectedMainboard = CardRows({
                                                      {"algae gharial",                  1},
                                                      {"cardthatdoesnotexistincardsxml", 3}
                                              });
        CardRows expectedSideboard = CardRows({
                                                      {"phelddagrif", 2}
                                              });

        ASSERT_EQ(expectedMainboard, decklistBuilder.mainboard());
        ASSERT_EQ(expectedSideboard, decklistBuilder.sideboard());
    }

    TEST(LoadingFromClipboardTest, IfMultipleScatteredBlanksAllMainBoard) {
        QString *clipboard = new QString(
                "1x Algae Gharial\n"
                        "3x CardThatDoesNotExistInCardsXml\n"
                        "\n"
                        "2x Phelddagrif\n"
                        "\n"
                        "3 Giant Growth\n"
        );

        DeckList *deckList = fromClipboard(clipboard);

        DecklistBuilder decklistBuilder = DecklistBuilder();
        deckList->forEachCard(decklistBuilder);

        CardRows expectedMainboard = CardRows({
                                                      {"algae gharial",                  1},
                                                      {"cardthatdoesnotexistincardsxml", 3},
                                                      {"phelddagrif", 2},
                                                      {"giant growth", 3}
                                              });
        CardRows expectedSideboard = CardRows({});

        ASSERT_EQ(expectedMainboard, decklistBuilder.mainboard());
        ASSERT_EQ(expectedSideboard, decklistBuilder.sideboard());
    }

    TEST(LoadingFromClipboardTest, LotsOfStuffInBulkTesting) {
        QString *clipboard = new QString(
                "\n"
                        "\n"
                        "\n"
                        "1x test1\n"
                        "testNoValueMB\n"
                        "2x test2\n"
                        "SB: 10 testSB\n"
                        "3 test3\n"
                        "4X test4\n"
                        "\n"
                        "\n"
                        "\n"
                        "\n"
                        "5x test5\n"
                        "6X test6\n"
                        "testNoValueSB\n"
                        "\n"
                        "\n"
                        "\n"
                        "\n"
        );

        DeckList *deckList = fromClipboard(clipboard);

        DecklistBuilder decklistBuilder = DecklistBuilder();
        deckList->forEachCard(decklistBuilder);

        CardRows expectedMainboard = CardRows({
                                                      {"test1", 1},
                                                      {"test2", 2},
                                                      {"test3", 3},
                                                      {"test4", 4},
                                                      {"testnovaluemb", 1}

                                      });
        CardRows expectedSideboard = CardRows({
                                                      {"testsb", 10},
                                                      {"test5", 5},
                                                      {"test6", 6},
                                                      {"testnovaluesb", 1}

                                              });
        ASSERT_EQ(expectedMainboard, decklistBuilder.mainboard());
        ASSERT_EQ(expectedSideboard, decklistBuilder.sideboard());
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}