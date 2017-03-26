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

using Cardrows = QMap<QString, int>;

struct AssertDeckList {
    Cardrows expectedMainboard;
    Cardrows expectedSideboard;

    Cardrows actualMainboard;
    Cardrows actualSideboard;

    AssertDeckList(Cardrows _expectedMainboardMap, Cardrows _expectedSideboard) :
            expectedMainboard(_expectedMainboardMap), expectedSideboard(_expectedSideboard) {}

    AssertDeckList(Cardrows _expectedMainboardMap) : AssertDeckList(_expectedMainboardMap, Cardrows({})) {}

    void operator()(const InnerDecklistNode *innerDecklistNode, const DecklistCardNode *card) {
        if (innerDecklistNode->getName() == "main") {
            actualMainboard[card->getName()] += card->getNumber();
        } else if (innerDecklistNode->getName() == "side") {
            actualSideboard[card->getName()] += card->getNumber();
        } else {
            FAIL();
        }
    }

    void makeAssertions() {
        ASSERT_EQ(expectedSideboard, actualSideboard);
        ASSERT_EQ(expectedMainboard, actualMainboard);
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
        QString *clipboard = new QString("");
        clipboard->append("1 Mountain\n");
        clipboard->append("2x Island\n");
        DeckList *deckList = fromClipboard(clipboard);

        Cardrows expectedMainboard = Cardrows({{"Mountain", 1},
                                               {"Island",   2}});
        AssertDeckList assertDeckList(expectedMainboard);

        SCOPED_TRACE("");
        deckList->forEachCard(assertDeckList);

        assertDeckList.makeAssertions();
    }

    TEST(LoadingFromClipboardTest, CommentsAreIgnoed) {
        QString *clipboard = new QString("");
        clipboard->append("// 1 Mountain\n");
        clipboard->append("// 2x Island\n");
        // TODO: Sideboard comments are ignored
        DeckList *deckList = fromClipboard(clipboard);

        Cardrows expectedMainboard = Cardrows({});
        AssertDeckList assertDeckList(expectedMainboard);

        SCOPED_TRACE("");
        deckList->forEachCard(assertDeckList);

        assertDeckList.makeAssertions();
    }

    TEST(LoadingFromClipboardTest, SideboardPrefix) {
        QString *clipboard = new QString("");
        clipboard->append("1 Mountain\n");
        clipboard->append("SB: 1 Mountain\n");
        clipboard->append("SB: 2x Island\n");
        DeckList *deckList = fromClipboard(clipboard);

        Cardrows expectedMainboard = Cardrows({{"Mountain", 1}});
        Cardrows expectedSideboard = Cardrows({{"Mountain", 1},
                                               {"Island",   2}});

        AssertDeckList assertDeckList(expectedMainboard, expectedSideboard);

        SCOPED_TRACE("");
        deckList->forEachCard(assertDeckList);

        assertDeckList.makeAssertions();
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}