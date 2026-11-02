#include "client/network/parsers/deck_link_to_api_transformer.h"
#include "client/network/parsers/manabox_json_deck_parser.h"

#include <QJsonDocument>
#include <gtest/gtest.h>
#include <libcockatrice/deck_list/tree/deck_list_card_node.h>
#include <libcockatrice/deck_list/tree/inner_deck_list_node.h>

TEST(ManaBoxImport, PublicShareUrlUsesCloudEndpointAndRejectsLookalikes)
{
    ParsedDeckInfo info;
    ASSERT_TRUE(DeckLinkToApiTransformer::parseDeckUrl(
        " https://www.manabox.app/decks/OqUd1vrIRD--RJGH4ZfedQ/?source=share#deck ", info));
    EXPECT_EQ(info.provider, DeckProvider::ManaBox);
    EXPECT_EQ(info.fullUrl, "https://cloud.manabox.app/decks/OqUd1vrIRD--RJGH4ZfedQ");
    EXPECT_FALSE(DeckLinkToApiTransformer::parseDeckUrl("https://manabox.app.evil/decks/id", info));
    EXPECT_FALSE(DeckLinkToApiTransformer::parseDeckUrl("https://evil/decks/manabox.app/decks/id", info));
    EXPECT_FALSE(DeckLinkToApiTransformer::parseDeckUrl("https://manabox.app/decks/id/extra", info));
    EXPECT_FALSE(DeckLinkToApiTransformer::parseDeckUrl("https://user@manabox.app/decks/id", info));
}

TEST(ManaBoxImport, PreservesBoardsPartnersQuantitiesAndDistinctPrintings)
{
    // Reduced public cloud response schema, with two different printings of Plains.
    const auto input = QJsonDocument::fromJson(R"json({
        "name": "Party Time",
        "cards": [
            {"name": "Plains", "quantity": 2, "boardCategory": 0, "setId": "clb",
             "collectorNumber": "451", "scryfallId": "printing-one"},
            {"name": "Plains", "quantity": 1, "boardCategory": 0, "setId": "clb",
             "collectorNumber": "452", "scryfallId": "printing-two"},
            {"name": "Despark", "quantity": 1, "boardCategory": 1, "setId": "moc",
             "collectorNumber": "322", "scryfallId": "side-printing"},
            {"name": "Burakos, Party Leader", "quantity": 1, "boardCategory": 2,
             "setId": "clb", "collectorNumber": "653", "scryfallId": "commander-one"},
            {"name": "Folk Hero", "quantity": 1, "boardCategory": 2, "setId": "clb",
             "collectorNumber": "650", "scryfallId": "commander-two"},
            {"name": "Adept Watershaper", "quantity": 1, "boardCategory": 3}
        ]
    })json")
                           .object();
    ManaBoxJsonParser parser;
    const DeckList deck = parser.parse(input);
    int mainCount = 0;
    int sideCount = 0;
    int plainsPrintings = 0;
    deck.forEachCard([&](InnerDecklistNode *zone, DecklistCardNode *card) {
        EXPECT_NE(card->getName(), "Adept Watershaper");
        if (zone->getName() == DECK_ZONE_MAIN) {
            mainCount += card->getNumber();
        } else if (zone->getName() == DECK_ZONE_SIDE) {
            sideCount += card->getNumber();
            EXPECT_EQ(card->getName(), "Despark");
        }
        if (card->getName() == "Plains") {
            ++plainsPrintings;
            EXPECT_EQ(card->getCardSetShortName(), "CLB");
            if (card->getCardProviderId() == "printing-one") {
                EXPECT_EQ(card->getNumber(), 2);
                EXPECT_EQ(card->getCardCollectorNumber(), "451");
            } else {
                EXPECT_EQ(card->getCardProviderId(), "printing-two");
                EXPECT_EQ(card->getNumber(), 1);
                EXPECT_EQ(card->getCardCollectorNumber(), "452");
            }
        }
    });
    EXPECT_EQ(mainCount, 5);
    EXPECT_EQ(sideCount, 1);
    EXPECT_EQ(plainsPrintings, 2);
    EXPECT_EQ(deck.getName(), "Party Time");
    EXPECT_EQ(deck.getBannerCard().name, "Burakos, Party Leader");
    EXPECT_EQ(deck.getBannerCard().providerId, "commander-one");
}

TEST(ManaBoxImport, MalformedOrOversizedPlayableRowsCannotProduceAPartialDeck)
{
    ManaBoxJsonParser parser;
    for (const char *row : {
             R"({"name":"Island","quantity":0,"boardCategory":0})",
             R"({"name":"Island","quantity":1.5,"boardCategory":0})",
             R"({"name":"","quantity":1,"boardCategory":0})",
             R"({"name":"Island","quantity":1,"boardCategory":9})",
             R"({"name":"Island","quantity":2147483647,"boardCategory":0})"}) {
        QJsonArray cards;
        cards.append(QJsonObject{{"name", "Plains"}, {"quantity", 1}, {"boardCategory", 0}});
        cards.append(QJsonDocument::fromJson(row).object());
        EXPECT_TRUE(parser.parse(QJsonObject{{"cards", cards}}).getCardList().isEmpty());
    }
    EXPECT_TRUE(parser.parse(QJsonObject{{"error", "Deck not found"}}).getCardList().isEmpty());
}
