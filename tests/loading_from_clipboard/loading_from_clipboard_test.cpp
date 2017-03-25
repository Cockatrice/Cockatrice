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

struct AssertDeckList {
    QList<QString> expectedCards;
    QList<QString> actualCards;

    AssertDeckList(QList<QString> _expectedCards) : expectedCards(_expectedCards) {}

    void operator()(const InnerDecklistNode *, const DecklistCardNode *card) {
        for (int i = 0; i < card->getNumber(); i++) {
            actualCards.append(card->getName());
        }
    }

    void makeAssertions() {
        ASSERT_EQ(expectedCards.size(), actualCards.size());
        ASSERT_EQ(expectedCards, actualCards);
    }
};

namespace {
    TEST(LoadingFromClipboardTest, EmptyDeck) {
        DeckList *deckList = fromClipboard(new QString(""));

        QStringList cardsInDeck = deckList->getCardList();
        ASSERT_TRUE(cardsInDeck.isEmpty()) << "Deck should be empty";
    }

    TEST(LoadingFromClipboardTest, EmptySideboard) {
        DeckList *deckList = fromClipboard(new QString("Sideboard"));

        QStringList cardsInDeck = deckList->getCardList();
        ASSERT_TRUE(cardsInDeck.isEmpty()) << "Deck should be empty";
    }

    TEST(LoadingFromClipboardTest, QuantityPrefixed) {
        QString * clipboard = new QString("");
        clipboard->append("1 Mountain\n");
        clipboard->append("2x Island\n");
        DeckList *deckList = fromClipboard(clipboard);

        QList<QString> expectedDeck = QList<QString>() << "Mountain" << "Island" << "Island";
        AssertDeckList assertDeckList(expectedDeck);

        deckList->forEachCard(assertDeckList);

        assertDeckList.makeAssertions();
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}