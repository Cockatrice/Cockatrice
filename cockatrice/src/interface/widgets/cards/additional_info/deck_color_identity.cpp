#include "deck_color_identity.h"

#include <QSet>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/deck_list/tree/inner_deck_list_node.h>

QString getDeckColorIdentity(const DeckList &deck)
{
    const QStringList cardList = deck.getCardList({DECK_ZONE_MAIN, DECK_ZONE_SIDE});
    if (cardList.isEmpty()) {
        return {};
    }

    QSet<QChar> colorSet; // A set to collect unique color symbols (e.g., W, U, B, R, G)

    for (const QString &cardName : cardList) {
        CardInfoPtr currentCard = CardDatabaseManager::query()->getCardInfo(cardName);
        if (currentCard) {
            const QString colors = currentCard->getColors(); // returns something like "WUB"
            for (const QChar &color : colors) {
                colorSet.insert(color);
            }
        }
    }

    // Ensure the color identity is in WUBRG order
    QString colorIdentity;
    const QString wubrgOrder = "WUBRG";
    for (const QChar &color : wubrgOrder) {
        if (colorSet.contains(color)) {
            colorIdentity.append(color);
        }
    }

    return colorIdentity;
}
