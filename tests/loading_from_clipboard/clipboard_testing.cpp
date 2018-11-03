#include "clipboard_testing.h"
#include <QTextStream>

void Result::operator()(const InnerDecklistNode *innerDecklistNode, const DecklistCardNode *card)
{
    if (innerDecklistNode->getName() == DECK_ZONE_MAIN) {
        mainboard.append({card->getName().toStdString(), card->getNumber()});
    } else if (innerDecklistNode->getName() == DECK_ZONE_SIDE) {
        sideboard.append({card->getName().toStdString(), card->getNumber()});
    } else {
        FAIL();
    }
}

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

    Result decklistBuilder;
    deckList.forEachCard(decklistBuilder);

    ASSERT_EQ(result.mainboard, decklistBuilder.mainboard);
    ASSERT_EQ(result.sideboard, decklistBuilder.sideboard);
}
