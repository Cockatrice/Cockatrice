#include "commander_spellbook_deck_request.h"

#include <QDebug>
#include <QJsonArray>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/deck_list/tree/deck_list_card_node.h>

void CommanderSpellbookDeckRequest::fromJson(const QJsonObject &json)
{
    mainDeck.clear();
    commanderDeck.clear();

    // Main deck
    const QJsonArray mainArray = json.value("main").toArray();
    for (const QJsonValue &value : mainArray) {
        if (!value.isObject()) {
            continue;
        }

        CardInDeckRequest card;
        card.fromJson(value.toObject());
        mainDeck.append(card);

        // Max size allowed by commanderspellbook
        if (mainDeck.size() >= 600) {
            break;
        }
    }

    // Commanders
    const QJsonArray commanderArray = json.value("commanders").toArray();
    for (const QJsonValue &value : commanderArray) {
        if (!value.isObject()) {
            continue;
        }

        CardInDeckRequest card;
        card.fromJson(value.toObject());
        commanderDeck.append(card);

        // Max size allowed by commanderspellbook
        if (commanderDeck.size() >= 12) {
            break;
        }
    }
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

void CommanderSpellbookDeckRequest::fromDeckList(const DeckList &deck)
{
    mainDeck.clear();
    commanderDeck.clear();

    // --- Mainboard ---
    const auto mainCards = deck.getCardNodes({DECK_ZONE_MAIN});
    for (const DecklistCardNode *node : mainCards) {
        if (!node) {
            continue;
        }

        CardInDeckRequest req;
        QJsonObject json;
        json.insert("card", node->getName());
        json.insert("quantity", node->getNumber());
        req.fromJson(json);

        mainDeck.append(req);

        // Max size allowed by commanderspellbook
        if (mainDeck.size() >= 600) {
            break;
        }
    }

    // --- Commander (bannerCard) ---
    const auto &metadata = deck.getMetadata();
    if (!metadata.bannerCard.name.isEmpty()) {
        CardInDeckRequest commander;
        QJsonObject json;
        json.insert("card", metadata.bannerCard.name);
        json.insert("quantity", 1);
        commander.fromJson(json);

        commanderDeck.append(commander);
    }
}

void CommanderSpellbookDeckRequest::debugPrint() const
{
    qDebug() << "Main deck:";
    for (const CardInDeckRequest &card : mainDeck) {
        card.debugPrint();
    }

    qDebug() << "Commanders:";
    for (const CardInDeckRequest &card : commanderDeck) {
        card.debugPrint();
    }
}
