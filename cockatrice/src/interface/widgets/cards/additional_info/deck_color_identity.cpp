#include "deck_color_identity.h"

#include "libcockatrice/card/card_info.h"
#include <libcockatrice/card/essence_types.h>

#include <QChar>
#include <QList>
#include <QSet>
#include <QStringList>
#include <libcockatrice/card/database/card_database_querier.h>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/deck_list/tree/inner_deck_list_node.h>

QString getDeckColorIdentity(const DeckList &deck, const CardDatabaseQuerier *db)
{
    const QStringList cardList = deck.getCardList({DECK_ZONE_MAIN, DECK_ZONE_SIDE});
    if (cardList.isEmpty()) {
        return {};
    }

    QSet<QChar> colorSet; // Unique Essence symbols (including N for colorless).

    for (const QString &cardName : cardList) {
        CardInfoPtr currentCard = db->getCardInfo(cardName);
        if (currentCard) {
            const QString colors = currentCard->getColors(); // returns something like "WUB"
            for (const QChar &color : colors) {
                colorSet.insert(color);
            }
        }
    }

    // Canonical Essence order is shared by the database and all widgets.
    QString colorIdentity;
    for (const QChar &color : Essence::symbols()) {
        if (colorSet.contains(color)) {
            colorIdentity.append(color);
        }
    }

    return colorIdentity;
}
