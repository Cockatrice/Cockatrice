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
        if (innerDecklistNode->getName() == "main") {
            actualMainboard[card->getName()] += card->getNumber();
        } else if (innerDecklistNode->getName() == "side") {
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
    TEST(LoadingFromClipboardTest, EmptyDeck) {
        DeckList *deckList = fromClipboard(new QString(""));
        ASSERT_TRUE(deckList->getCardList().isEmpty()) << "Deck should be empty";
    }

    TEST(LoadingFromClipboardTest, EmptySideboard) {
        DeckList *deckList = fromClipboard(new QString("Sideboard"));
        ASSERT_TRUE(deckList->getCardList().isEmpty()) << "Deck should be empty";
    }

    TEST(LoadingFromClipboardTest, QuantityPrefixed) {
        QString *clipboard = new QString(
                "1 Mountain\n"
                "2x Island\n"
        );
        DeckList *deckList = fromClipboard(clipboard);

        DecklistBuilder decklistBuilder = DecklistBuilder();
        deckList->forEachCard(decklistBuilder);

        CardRows expectedMainboard = CardRows({{"Mountain", 1},
                                               {"Island",   2}});
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

        CardRows expectedMainboard = CardRows({{"Mountain", 1}});
        CardRows expectedSideboard = CardRows({{"Mountain", 1},
                                               {"Island",   2}});

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

        CardRows expectedMainboard = CardRows({{"CardThatDoesNotExistInCardsXml", 1}});
        CardRows expectedSideboard = CardRows({});

        ASSERT_EQ(expectedMainboard, decklistBuilder.mainboard());
        ASSERT_EQ(expectedSideboard, decklistBuilder.sideboard());
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}