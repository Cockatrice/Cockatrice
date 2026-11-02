#include "manabox_json_deck_parser.h"

#include <libcockatrice/deck_list/tree/deck_list_card_node.h>

DeckList ManaBoxJsonParser::parse(const QJsonObject &obj)
{
    DeckList deckList;
    if (!obj.value("cards").isArray()) {
        return deckList;
    }

    const CardNameNormalizer normalizeName;
    bool hasCommander = false;
    for (const auto &value : obj.value("cards").toArray()) {
        const QJsonObject card = value.toObject();
        // ManaBox boards: 0 main, 1 side, 2 commanders, 3 maybeboard.
        // Unknown boards or malformed playable rows must not produce a partial deck.
        const int board = card.value("boardCategory").toInt(-1);
        if (board < 0 || board > 3) {
            return DeckList();
        }
        if (board == 3) {
            continue;
        }

        const QString name = normalizeName(card.value("name").toString().trimmed());
        const int quantity = card.value("quantity").toInt();
        if (name.isEmpty() || quantity <= 0) {
            return DeckList();
        }
        const QString set = card.value("setId").toString().toUpper();
        const QString collectorNumber = card.value("collectorNumber").toString();
        const QString providerId = card.value("scryfallId").toString();
        const QString zone = board == 1 ? DECK_ZONE_SIDE : DECK_ZONE_MAIN;
        const auto *node = deckList.addCards(name, quantity, zone, -1, set, collectorNumber, providerId);
        if (!node || node->getNumber() != quantity) {
            return DeckList();
        }

        // All partner commanders are playable; the first supplies the deck banner.
        if (board == 2 && !hasCommander) {
            deckList.setBannerCard({name, providerId});
            hasCommander = true;
        }
    }

    deckList.setName(obj.value("name").toString());
    deckList.setComments(obj.value("description").toString());
    return deckList;
}
