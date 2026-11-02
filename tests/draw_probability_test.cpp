#include "interface/widgets/deck_analytics/analyzer_modules/draw_probability/draw_probability_calculator.h"
#include "interface/widgets/deck_analytics/analyzer_modules/draw_probability/draw_probability_config.h"

#include <QJsonObject>
#include <cmath>
#include <gtest/gtest.h>
#include <libcockatrice/card/database/card_database.h>
#include <libcockatrice/card/database/card_database_querier.h>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/deck_list/tree/deck_list_card_node.h>
#include <libcockatrice/deck_list/tree/inner_deck_list_node.h>
#include <limits>

namespace
{
void addCard(CardDatabase &database,
             const QString &name,
             const QString &type,
             const QString &mainType,
             const QString &manaValue = "1")
{
    auto card = CardInfo::newInstance(name);
    card->setCardType(type);
    card->setProperty("maintype", mainType);
    card->setProperty("cmc", manaValue);
    database.addCard(card);
}

long double choose(int n, int k)
{
    if (k < 0 || k > n) {
        return 0;
    }
    long double result = 1;
    for (int i = 1; i <= k; ++i) {
        result = result * (n - k + i) / i;
    }
    return result;
}
} // namespace

TEST(DrawProbability, UsesOnlyTheMainDeckAndCombinesPrintings)
{
    CardDatabase database;
    addCard(database, "Bolt", "Instant", "Instant");
    addCard(database, "Land", "Basic Land - Plains", "Land", "0");
    DeckList deck;
    deck.addCard("Bolt", DECK_ZONE_MAIN, -1, "SET1")->setNumber(3);
    deck.addCard("Bolt", DECK_ZONE_MAIN, -1, "SET2")->setNumber(1);
    deck.addCard("Land", DECK_ZONE_MAIN)->setNumber(56);
    deck.addCard("Bolt", DECK_ZONE_SIDE)->setNumber(15);
    deck.addCard("Commander", "command")->setNumber(1);
    deck.addCard("Token", "tokens")->setNumber(20);
    const auto population = collectDrawProbabilityPopulation(deck, *database.query(), "name");
    EXPECT_EQ(60, population.totalCards);
    EXPECT_EQ(0, population.unclassifiedCards);
    ASSERT_EQ(2, population.categoryCounts.size());
    EXPECT_EQ(4, population.categoryCounts.value("Bolt"));
    EXPECT_NEAR(4.0 / 60, *calculateDrawProbability(population.totalCards, 4, 1, 1, false), 1e-12);
}

TEST(DrawProbability, UnknownCardsRemainInPopulationAndNameOdds)
{
    CardDatabase database;
    addCard(database, "Known", "Creature - Elf", "Creature");
    DeckList deck;
    deck.addCard("Known", DECK_ZONE_MAIN)->setNumber(4);
    deck.addCard("Unknown", DECK_ZONE_MAIN)->setNumber(56);
    const auto names = collectDrawProbabilityPopulation(deck, *database.query(), "name");
    EXPECT_EQ(60, names.totalCards);
    EXPECT_EQ(0, names.unclassifiedCards);
    EXPECT_EQ(56, names.categoryCounts.value("Unknown"));
    EXPECT_NEAR(4.0 / 60, *calculateDrawProbability(names.totalCards, names.categoryCounts.value("Known"), 1, 1, false),
                1e-12);
    const auto types = collectDrawProbabilityPopulation(deck, *database.query(), "type");
    EXPECT_EQ(60, types.totalCards);
    EXPECT_EQ(56, types.unclassifiedCards);
}

TEST(DrawProbability, SubtypesExcludeTypesAndPreserveHyphenatedNames)
{
    CardDatabase database;
    addCard(database, "Worker", "Artifact Creature - Assembly-Worker", "Creature");
    addCard(database, "Two Faces", QString::fromUtf8("Legendary Creature — Elf Druid // Creature — Elf Wizard"),
            "Creature");
    addCard(database, "Spell", "Sorcery", "Sorcery");
    DeckList deck;
    deck.addCard("Worker", DECK_ZONE_MAIN)->setNumber(2);
    deck.addCard("Two Faces", DECK_ZONE_MAIN)->setNumber(3);
    deck.addCard("Spell", DECK_ZONE_MAIN)->setNumber(4);
    const auto population = collectDrawProbabilityPopulation(deck, *database.query(), "subtype");
    EXPECT_EQ(9, population.totalCards);
    EXPECT_EQ(0, population.unclassifiedCards);
    EXPECT_EQ(4, population.categoryCounts.size());
    EXPECT_EQ(2, population.categoryCounts.value("Assembly-Worker"));
    EXPECT_EQ(3, population.categoryCounts.value("Elf"));
    EXPECT_EQ(3, population.categoryCounts.value("Druid"));
    EXPECT_EQ(3, population.categoryCounts.value("Wizard"));
    EXPECT_FALSE(population.categoryCounts.contains("Creature"));
    EXPECT_FALSE(population.categoryCounts.contains("Sorcery"));
}

TEST(DrawProbability, CategoriesCountEachPhysicalCardOnlyOnce)
{
    CardDatabase database;
    addCard(database, "Repeating Type", "Creature - Elf", "Creature Creature");
    DeckList deck;
    deck.addCard("Repeating Type", DECK_ZONE_MAIN)->setNumber(4);
    const auto population = collectDrawProbabilityPopulation(deck, *database.query(), "type");
    EXPECT_EQ(4, population.categoryCounts.value("Creature"));
}

TEST(DrawProbability, ManaValuePreservesFractionsAndRejectsMissingValues)
{
    CardDatabase database;
    addCard(database, "Half", "Creature", "Creature", "0.5");
    addCard(database, "Zero", "Land", "Land", "0");
    addCard(database, "Missing", "Creature", "Creature", "");
    DeckList deck;
    deck.addCard("Half", DECK_ZONE_MAIN)->setNumber(2);
    deck.addCard("Zero", DECK_ZONE_MAIN)->setNumber(3);
    deck.addCard("Missing", DECK_ZONE_MAIN)->setNumber(4);
    const auto population = collectDrawProbabilityPopulation(deck, *database.query(), "cmc");
    EXPECT_EQ(2, population.categoryCounts.value("0.5"));
    EXPECT_EQ(3, population.categoryCounts.value("0"));
    EXPECT_EQ(4, population.unclassifiedCards);
    EXPECT_EQ(9, population.totalCards);
}

TEST(DrawProbability, CountsLargeDecksWithoutIntegerOverflow)
{
    CardDatabase database;
    DeckList deck;
    deck.addCard("A", DECK_ZONE_MAIN)->setNumber(std::numeric_limits<int>::max());
    deck.addCard("B", DECK_ZONE_MAIN)->setNumber(std::numeric_limits<int>::max());
    deck.addCard("Invalid", DECK_ZONE_MAIN)->setNumber(-1);
    const auto population = collectDrawProbabilityPopulation(deck, *database.query(), "name");
    EXPECT_EQ(2LL * std::numeric_limits<int>::max(), population.totalCards);
    EXPECT_NEAR(0.5,
                *calculateDrawProbability(population.totalCards, population.categoryCounts.value("A"), 1, 1, false),
                1e-12);
}

TEST(DrawProbability, InvalidExperimentsAreDistinctFromImpossibleOutcomes)
{
    EXPECT_FALSE(calculateDrawProbability(60, 4, 61, 1, false));
    EXPECT_FALSE(calculateDrawProbability(60, 61, 7, 1, false));
    EXPECT_FALSE(calculateDrawProbability(60, -1, 7, 1, false));
    EXPECT_FALSE(calculateDrawProbability(60, 4, -1, 1, false));
    EXPECT_FALSE(calculateDrawProbability(60, 4, 7, -1, false));
    EXPECT_EQ(0.0, calculateDrawProbability(60, 4, 7, 5, false));
    EXPECT_EQ(0.0, calculateDrawProbability(60, 59, 7, 1, false));
    EXPECT_EQ(1.0, calculateDrawProbability(60, 59, 7, 6, true));
    EXPECT_EQ(1.0, calculateDrawProbability(0, 0, 0, 0, false));
    EXPECT_EQ(0.0, calculateDrawProbability(0, 0, 0, 1, true));
}

TEST(DrawProbability, MatchesCombinatorialDistributionAcrossItsEntireSupport)
{
    for (int total = 0; total <= 16; ++total) {
        for (int copies = 0; copies <= total; ++copies) {
            for (int drawn = 0; drawn <= total; ++drawn) {
                long double tail = 0;
                for (int quantity = drawn + 1; quantity >= 0; --quantity) {
                    const long double exact =
                        choose(copies, quantity) * choose(total - copies, drawn - quantity) / choose(total, drawn);
                    tail += exact;
                    const auto actualExact = calculateDrawProbability(total, copies, drawn, quantity, false);
                    const auto actualTail = calculateDrawProbability(total, copies, drawn, quantity, true);
                    ASSERT_TRUE(actualExact);
                    ASSERT_TRUE(actualTail);
                    EXPECT_NEAR(static_cast<double>(exact), *actualExact, 1e-12);
                    EXPECT_NEAR(static_cast<double>(tail), *actualTail, 1e-12);
                }
            }
        }
    }
}

TEST(DrawProbability, NormalizesConfigurationAndAllowsNoSuccesses)
{
    const auto config = DrawProbabilityConfig::fromJson(
        {{"criteria", "nonsense"}, {"quantity", -2}, {"drawn", 999}, {"atLeast", false}});
    EXPECT_EQ("name", config.criteria);
    EXPECT_EQ(0, config.quantity);
    EXPECT_EQ(60, config.drawn);
    EXPECT_FALSE(config.atLeast);
    const auto roundTrip = DrawProbabilityConfig::fromJson(config.toJson());
    EXPECT_EQ(config.toJson(), roundTrip.toJson());
    EXPECT_GT(*calculateDrawProbability(60, 4, 7, 0, false), 0);
}
