#include "clipboard_testing.h"

#include <QTextStream>
#include <libcockatrice/deck_list/tree/deck_list_card_node.h>

DeckList getDeckList(const QString &clipboard)
{
    DeckList deckList;
    QString cp(clipboard);
    QTextStream stream(&cp); // text stream requires local copy
    deckList.loadFromStream_Plain(stream, false);
    return deckList;
}

void testEmpty(const QString &clipboard)
{
    DeckList deckList = getDeckList(clipboard);

    ASSERT_TRUE(deckList.getCardList().isEmpty());
}

void testHash(const QString &clipboard, const std::string &hash)
{
    DeckList deckList = getDeckList(clipboard);

    ASSERT_EQ(deckList.getDeckHash().toStdString(), hash);
}

void testDeck(const QString &clipboard, const Result &result)
{
    DeckList deckList = getDeckList(clipboard);

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
