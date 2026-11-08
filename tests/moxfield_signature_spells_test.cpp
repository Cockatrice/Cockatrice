#include "client/network/parsers/interface_json_deck_parser.h"

#include <QJsonDocument>
#include <gtest/gtest.h>
#include <libcockatrice/deck_list/tree/deck_list_card_node.h>
#include <libcockatrice/deck_list/tree/inner_deck_list_node.h>

TEST(MoxfieldImport, SignatureSpellIsAddedToMainWithoutReplacingCommanderBanner)
{
    // Moxfield v2 board objects map IDs to entries with quantity and card.
    const auto input = QJsonDocument::fromJson(R"json({
        "name": "Oathbreaker example", "description": "Notes retained",
        "mainboard": {"land": {"quantity": 1, "card": {"name": "Island", "set": "abc", "cn": "1"}}},
        "sideboard": {"side": {"quantity": 1, "card": {"name": "Mountain", "set": "abc", "cn": "2"}}},
        "commanders": {"walker": {"quantity": 1, "card": {"name": "Jace", "set": "abc", "cn": "3", "scryfall_id": "walker-id"}}},
        "signatureSpells": {"spell": {"quantity": 1, "card": {"name": "Lightning Bolt", "set": "xyz", "cn": "4", "scryfall_id": "spell-id"}}}
    })json")
                           .object();
    MoxfieldJsonParser parser;
    DeckList deck = parser.parse(input);
    int mainSpellCount = 0;
    int sideSpellCount = 0;
    int mainLands = 0;
    int sideLands = 0;
    deck.forEachCard([&](InnerDecklistNode *zone, DecklistCardNode *card) {
        if (card->getName() == "Island" && zone->getName() == DECK_ZONE_MAIN) {
            mainLands += card->getNumber();
        }
        if (card->getName() == "Mountain" && zone->getName() == DECK_ZONE_SIDE) {
            sideLands += card->getNumber();
        }
        if (card->getName() != "Lightning Bolt") {
            return;
        }
        if (zone->getName() == DECK_ZONE_MAIN) {
            mainSpellCount += card->getNumber();
            EXPECT_EQ(card->getCardSetShortName(), "XYZ");
            EXPECT_EQ(card->getCardCollectorNumber(), "4");
            EXPECT_EQ(card->getCardProviderId(), "spell-id");
        } else if (zone->getName() == DECK_ZONE_SIDE) {
            sideSpellCount += card->getNumber();
        }
    });
    EXPECT_EQ(mainSpellCount, 1);
    EXPECT_EQ(sideSpellCount, 0);
    EXPECT_EQ(mainLands, 1);
    EXPECT_EQ(sideLands, 1);
    EXPECT_EQ(deck.getName(), "Oathbreaker example");
    EXPECT_EQ(deck.getComments(), "Notes retained");
    EXPECT_EQ(deck.getBannerCard().name, "Jace");
    EXPECT_EQ(deck.getBannerCard().providerId, "walker-id");
}

TEST(MoxfieldImport, QuantityAwareDeckInsertionRetainsCountAndPrinting)
{
    DeckList deck;
    auto *card = deck.addCards("Island", 2, DECK_ZONE_MAIN, -1, "ABC", "4", "id");
    ASSERT_NE(card, nullptr);
    EXPECT_EQ(card->getNumber(), 2);
    EXPECT_EQ(card->getCardProviderId(), "id");
    EXPECT_EQ(deck.addCards("Island", 0, DECK_ZONE_MAIN), nullptr);
}

TEST(OnlineImport, ArchidektPreservesNameDescriptionAndCards)
{
    const auto input = QJsonDocument::fromJson(R"json({
        "name": "Online deck", "description": "Deck notes",
        "cards": [{"quantity": 2, "card": {
            "oracleCard": {"name": "Island"}, "edition": {"editioncode": "abc"}, "collectorNumber": "1"
        }}]
    })json")
                           .object();
    ArchidektJsonParser parser;
    const DeckList deck = parser.parse(input);
    int count = 0;
    deck.forEachCard([&](InnerDecklistNode *zone, DecklistCardNode *card) {
        if (zone->getName() == DECK_ZONE_MAIN && card->getName() == "Island") {
            count += card->getNumber();
        }
    });
    EXPECT_EQ(count, 2);
    EXPECT_EQ(deck.getName(), "Online deck");
    EXPECT_EQ(deck.getComments(), "Deck notes");
}
