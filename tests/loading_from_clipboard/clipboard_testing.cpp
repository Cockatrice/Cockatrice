#include "clipboard_testing.h"

#include <QTextStream>
#include <libcockatrice/deck_list/deck_list_card_node.h>

void testEmpty(const QString &clipboard)
{
    QString cp(clipboard);
    DeckList deckList;
    QTextStream stream(&cp); // text stream requires local copy
    deckList.loadFromStream_Plain(stream, false);

    ASSERT_TRUE(deckList.getCardList().isEmpty());
}

void testDeck(const QString &clipboard, const Result &result)
{
    QString cp(clipboard);
    DeckList deckList;
    QTextStream stream(&cp); // text stream requires local copy
    deckList.loadFromStream_Plain(stream, false);

    ASSERT_EQ(result.name, deckList.getName().toStdString());
    ASSERT_EQ(result.comments, deckList.getComments().toStdString());

    CardRows mainboard;
    CardRows sideboard;

    auto extractCards = [&mainboard, &sideboard](const InnerDecklistNode *innerDecklistNode,
                                                 const DecklistCardNode *card) {
        if (innerDecklistNode->getName() == DECK_ZONE_MAIN) {
            mainboard.append({card->getName().toStdString(), card->getNumber()});
        } else if (innerDecklistNode->getName() == DECK_ZONE_SIDE) {
            sideboard.append({card->getName().toStdString(), card->getNumber()});
        } else {
            FAIL();
        }
    };

    deckList.forEachCard(extractCards);

    ASSERT_EQ(result.mainboard, mainboard);
    ASSERT_EQ(result.sideboard, sideboard);
}
