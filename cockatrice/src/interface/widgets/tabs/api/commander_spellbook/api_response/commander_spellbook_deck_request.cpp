#include "commander_spellbook_deck_request.h"

#include <QJsonArray>
#include <libcockatrice/deck_list/tree/deck_list_card_node.h>

CommanderSpellbookDeckRequest CommanderSpellbookDeckRequest::fromJson(const QJsonObject &json)
{
    CommanderSpellbookDeckRequest request;

    // Main deck
    const QJsonArray mainArray = json.value("main").toArray();
    for (const QJsonValue &value : mainArray) {
        if (!value.isObject()) {
            continue;
        }

        request.mainDeck.append(CardInDeckRequest::fromJson(value.toObject()));

        // Max size allowed by commanderspellbook
        if (request.mainDeck.size() >= 600) {
            break;
        }
    }

    // Commanders
    const QJsonArray commanderArray = json.value("commanders").toArray();
    for (const QJsonValue &value : commanderArray) {
        if (!value.isObject()) {
            continue;
        }

        request.commanderDeck.append(CardInDeckRequest::fromJson(value.toObject()));

        // Max size allowed by commanderspellbook
        if (request.commanderDeck.size() >= 12) {
            break;
        }
    }

    return request;
}

QJsonObject CommanderSpellbookDeckRequest::toJson() const
{
    QJsonObject json;

    QJsonArray mainArray;
    for (const CardInDeckRequest &card : mainDeck) {
        mainArray.append(card.toJson());
    }

    QJsonArray commanderArray;
    for (const CardInDeckRequest &card : commanderDeck) {
        commanderArray.append(card.toJson());
    }

    json.insert("main", mainArray);
    json.insert("commanders", commanderArray);

    return json;
}

CommanderSpellbookDeckRequest CommanderSpellbookDeckRequest::fromDeckList(const DeckList &deck)
{
    CommanderSpellbookDeckRequest request;

    // --- Mainboard ---
    const auto mainCards = deck.getCardNodes({DECK_ZONE_MAIN});
    for (const DecklistCardNode *node : mainCards) {
        if (!node) {
            continue;
        }

        QJsonObject json;
        json.insert("card", node->getName());
        json.insert("quantity", node->getNumber());
        request.mainDeck.append(CardInDeckRequest::fromJson(json));

        // Max size allowed by commanderspellbook
        if (request.mainDeck.size() >= 600) {
            break;
        }
    }

    // --- Commander (bannerCard) ---
    const auto &metadata = deck.getMetadata();
    if (!metadata.bannerCard.name.isEmpty()) {
        QJsonObject json;
        json.insert("card", metadata.bannerCard.name);
        json.insert("quantity", 1);
        request.commanderDeck.append(CardInDeckRequest::fromJson(json));
    }

    return request;
}
