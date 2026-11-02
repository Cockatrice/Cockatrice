#include "deck_color_identity.h"

#include "libcockatrice/card/card_info.h"
#include "libcockatrice/card/game_specific_terms.h"

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
    if (!db || cardList.isEmpty()) {
        return {};
    }

    QSet<QChar> colorSet; // A set to collect unique color symbols (e.g., W, U, B, R, G)

    for (const QString &cardName : cardList) {
        CardInfoPtr currentCard = db->getCardInfo(cardName);
        if (!currentCard) {
            // A partial database cannot establish the identity of the whole deck.
            return {};
        }

        // Oracle's identity includes colored rules symbols, other faces and devoid
        // cards. Older/custom databases may only provide the card's colors.
        const QString colors =
            (currentCard->hasProperty(Mtg::ColorIdentity) ? currentCard->getProperty(Mtg::ColorIdentity)
                                                          : currentCard->getColors())
                .trimmed()
                .toUpper();
        if (colors == QStringLiteral("C")) {
            continue;
        }
        for (const QChar &color : colors) {
            if (!QStringLiteral("WUBRG").contains(color)) {
                return {};
            }
            colorSet.insert(color);
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

    // Empty metadata is also used by old uploads and unloaded decks. Keep that
    // distinct from a deck whose known cards have no colors in their identity.
    return colorIdentity.isEmpty() ? QStringLiteral("C") : colorIdentity;
}
