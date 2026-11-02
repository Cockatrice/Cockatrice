#include "interface/widgets/visual_deck_editor/sample_hand_generator.h"

#include <QHash>
#include <gtest/gtest.h>
#include <libcockatrice/card/database/card_database.h>
#include <libcockatrice/card/database/card_database_querier.h>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/deck_list/tree/deck_list_card_node.h>
#include <libcockatrice/deck_list/tree/inner_deck_list_node.h>
#include <libcockatrice/interfaces/noop_card_preference_provider.h>
#include <limits>

namespace
{
void addCard(CardDatabase &database, const QString &name, const QString &manaValue)
{
    auto card = CardInfo::newInstance(name);
    card->setProperty("cmc", manaValue);
    database.addCard(card);
}

QHash<QString, int> countNames(const QList<ExactCard> &cards)
{
    QHash<QString, int> counts;
    for (const auto &card : cards) {
        ++counts[card.getName()];
    }
    return counts;
}
} // namespace

TEST(SampleHandGenerator, ExhaustingMainDeckPreservesEveryCopyAndUnknownName)
{
    NoopCardPreferenceProvider preferences;
    CardDatabase database(nullptr, &preferences);
    addCard(database, "Known", "2");
    DeckList deck;
    deck.addCard("Known", DECK_ZONE_MAIN)->setNumber(3);
    deck.addCard("Unknown", DECK_ZONE_MAIN)->setNumber(2);
    deck.addCard("Sideboard", DECK_ZONE_SIDE)->setNumber(10);
    deck.addCard("Token", DECK_ZONE_TOKENS)->setNumber(30);
    std::mt19937 random(42);
    const auto hand = generateSampleHand(deck, *database.query(), 99, random);
    ASSERT_EQ(5, hand.size());
    const auto counts = countNames(hand);
    EXPECT_EQ(3, counts.value("Known"));
    EXPECT_EQ(2, counts.value("Unknown"));
    EXPECT_FALSE(counts.contains("Sideboard"));
    EXPECT_FALSE(counts.contains("Token"));
    for (const auto &card : hand) {
        EXPECT_TRUE(static_cast<bool>(card));
    }
}

TEST(SampleHandGenerator, PartialDrawNeverExceedsAvailableCopies)
{
    CardDatabase database;
    DeckList deck;
    deck.addCard("One", DECK_ZONE_MAIN)->setNumber(1);
    deck.addCard("Two", DECK_ZONE_MAIN)->setNumber(2);
    deck.addCard("Many", DECK_ZONE_MAIN)->setNumber(57);
    std::mt19937 random(7);
    for (int iteration = 0; iteration < 100; ++iteration) {
        const auto hand = generateSampleHand(deck, *database.query(), 7, random);
        ASSERT_EQ(7, hand.size());
        const auto counts = countNames(hand);
        EXPECT_LE(counts.value("One"), 1);
        EXPECT_LE(counts.value("Two"), 2);
        EXPECT_LE(counts.value("Many"), 7);
    }
}

TEST(SampleHandGenerator, DrawsFromHugeCountsWithoutMaterializingTheDeck)
{
    CardDatabase database;
    DeckList deck;
    deck.addCard("A", DECK_ZONE_MAIN)->setNumber(std::numeric_limits<int>::max());
    deck.addCard("B", DECK_ZONE_MAIN)->setNumber(std::numeric_limits<int>::max());
    std::mt19937 random(123);
    const auto hand = generateSampleHand(deck, *database.query(), 99, random);
    EXPECT_EQ(99, hand.size());
    const auto counts = countNames(hand);
    EXPECT_GT(counts.value("A"), 0);
    EXPECT_GT(counts.value("B"), 0);
}

TEST(SampleHandGenerator, SortsNumericManaValuesAndPlacesMissingMetadataLast)
{
    NoopCardPreferenceProvider preferences;
    CardDatabase database(nullptr, &preferences);
    addCard(database, "Ten", "10");
    addCard(database, "Two", "2");
    addCard(database, "Half", "0.5");
    addCard(database, "Missing", "");
    DeckList deck;
    for (const auto &name : {"Ten", "Two", "Half", "Missing"}) {
        deck.addCard(name, DECK_ZONE_MAIN)->setNumber(1);
    }
    std::mt19937 random(4);
    const auto hand = generateSampleHand(deck, *database.query(), 4, random);
    ASSERT_EQ(4, hand.size());
    EXPECT_EQ("Half", hand[0].getName());
    EXPECT_EQ("Two", hand[1].getName());
    EXPECT_EQ("Ten", hand[2].getName());
    EXPECT_EQ("Missing", hand[3].getName());
}

TEST(SampleHandGenerator, EmptyAndInvalidCountsProduceNoPhantomCards)
{
    CardDatabase database;
    DeckList deck;
    std::mt19937 random(1);
    EXPECT_TRUE(generateSampleHand(deck, *database.query(), 7, random).isEmpty());
    deck.addCard("Zero", DECK_ZONE_MAIN)->setNumber(0);
    deck.addCard("Negative", DECK_ZONE_MAIN)->setNumber(-5);
    EXPECT_TRUE(generateSampleHand(deck, *database.query(), 7, random).isEmpty());
    deck.addCard("One", DECK_ZONE_MAIN)->setNumber(1);
    EXPECT_TRUE(generateSampleHand(deck, *database.query(), 0, random).isEmpty());
    EXPECT_TRUE(generateSampleHand(deck, *database.query(), -1, random).isEmpty());
    EXPECT_EQ(1, generateSampleHand(deck, *database.query(), 7, random).size());
}
