/**
 * @file deck_list_zones_test.cpp
 * @brief Tests for custom deck zones (deck-unique zones nested under a board zone).
 *
 * Custom zones allow players to organize cards within a board (e.g. "Removal" under
 * the mainboard) without changing the board semantics: cards in a custom zone under
 * "main" are still mainboard cards for hashing, sideboard size, legality and export.
 */

#include <QStringList>
#include <gtest/gtest.h>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/deck_list/deck_list_node_tree.h>
#include <libcockatrice/deck_list/tree/deck_list_card_node.h>
#include <libcockatrice/deck_list/tree/inner_deck_list_node.h>

namespace
{

/**
 * @brief Collects (board zone name, card node) pairs via forEachCard.
 */
struct BoardCardPair
{
    QString boardZone;
    QString cardName;
    int amount;
};

QList<BoardCardPair> collectBoardCardPairs(const DeckList &deck)
{
    QList<BoardCardPair> result;
    deck.forEachCard([&result](InnerDecklistNode *boardZone, DecklistCardNode *card) {
        result.append({boardZone->getName(), card->getName(), card->getNumber()});
    });
    return result;
}

bool hasPair(const QList<BoardCardPair> &pairs, const QString &boardZone, const QString &cardName)
{
    for (const auto &pair : pairs) {
        if (pair.boardZone == boardZone && pair.cardName == cardName) {
            return true;
        }
    }
    return false;
}

int totalCards(const QList<BoardCardPair> &pairs)
{
    int total = 0;
    for (const auto &pair : pairs) {
        total += pair.amount;
    }
    return total;
}

} // namespace

// =====================================================================================================================
// Zone creation
// =====================================================================================================================

TEST(DeckListZones, AddCustomZoneNestsUnderBoard)
{
    DeckList deck;
    auto *tree = deck.getTree();

    auto *zone = tree->addCustomZone(DECK_ZONE_MAIN, "Removal");
    ASSERT_NE(zone, nullptr);
    EXPECT_EQ(zone->getName(), QString("Removal"));
    ASSERT_NE(zone->getParent(), nullptr);
    EXPECT_EQ(zone->getParent()->getName(), QString(DECK_ZONE_MAIN));

    // The custom zone is nested, not a new top-level zone.
    auto topLevelZones = tree->getZoneNodes();
    QStringList topLevelNames;
    for (auto *node : topLevelZones) {
        topLevelNames.append(node->getName());
    }
    EXPECT_FALSE(topLevelNames.contains("Removal"));

    // It is discoverable through the board zone.
    auto customZones = tree->getCustomZones(DECK_ZONE_MAIN);
    ASSERT_EQ(customZones.size(), 1);
    EXPECT_EQ(customZones.first()->getName(), QString("Removal"));
}

TEST(DeckListZones, CustomZoneNamesAreDeckUnique)
{
    DeckList deck;
    auto *tree = deck.getTree();

    ASSERT_NE(tree->addCustomZone(DECK_ZONE_MAIN, "Removal"), nullptr);
    // Same name on a different board is rejected.
    EXPECT_EQ(tree->addCustomZone(DECK_ZONE_SIDE, "Removal"), nullptr);
    // A name that collides with a built-in board zone is rejected.
    EXPECT_EQ(tree->addCustomZone(DECK_ZONE_MAIN, DECK_ZONE_MAIN), nullptr);
    EXPECT_EQ(tree->addCustomZone(DECK_ZONE_MAIN, DECK_ZONE_SIDE), nullptr);
    EXPECT_EQ(tree->addCustomZone(DECK_ZONE_MAIN, DECK_ZONE_MAYBEBOARD), nullptr);
    EXPECT_EQ(tree->addCustomZone(DECK_ZONE_MAIN, DECK_ZONE_TOKENS), nullptr);
}

TEST(DeckListZones, AddCustomZoneUnknownBoardFails)
{
    DeckList deck;
    auto *tree = deck.getTree();

    EXPECT_EQ(tree->addCustomZone("not_a_board", "Removal"), nullptr);
}

TEST(DeckListZones, MaybeboardIsLazilyCreated)
{
    DeckList deck;
    auto *tree = deck.getTree();

    ASSERT_NE(tree->addCustomZone(DECK_ZONE_MAYBEBOARD, "Candidates"), nullptr);

    // The maybeboard board zone now exists, with the custom zone nested inside.
    auto customZones = tree->getCustomZones(DECK_ZONE_MAYBEBOARD);
    ASSERT_EQ(customZones.size(), 1);
    EXPECT_EQ(customZones.first()->getName(), QString("Candidates"));
}

// =====================================================================================================================
// Card placement
// =====================================================================================================================

TEST(DeckListZones, AddCardToCustomZoneKeepsBoardSemantics)
{
    DeckList deck;
    auto *tree = deck.getTree();

    ASSERT_NE(tree->addCustomZone(DECK_ZONE_MAIN, "Removal"), nullptr);
    tree->addCard("Lightning Bolt", 4, "Removal", -1);

    // The card is reported as a mainboard card.
    auto pairs = collectBoardCardPairs(deck);
    EXPECT_TRUE(hasPair(pairs, DECK_ZONE_MAIN, "Lightning Bolt"));
    EXPECT_FALSE(hasPair(pairs, DECK_ZONE_SIDE, "Lightning Bolt"));

    // It is physically nested inside the custom zone.
    auto customZones = tree->getCustomZones(DECK_ZONE_MAIN);
    ASSERT_EQ(customZones.size(), 1);
    ASSERT_EQ(customZones.first()->size(), 1);
    auto *card = dynamic_cast<const DecklistCardNode *>(customZones.first()->at(0));
    ASSERT_NE(card, nullptr);
    EXPECT_EQ(card->getName(), QString("Lightning Bolt"));
    EXPECT_EQ(card->getNumber(), 4);

    // Zone-scoped queries include it.
    EXPECT_TRUE(deck.getCardList({DECK_ZONE_MAIN}).contains("Lightning Bolt"));
    EXPECT_FALSE(deck.getCardList({DECK_ZONE_SIDE}).contains("Lightning Bolt"));
    EXPECT_EQ(deck.getCardNodes({DECK_ZONE_MAIN}).size(), 1);
}

TEST(DeckListZones, LegacyTopLevelZoneStillWorks)
{
    DeckList deck;
    auto *tree = deck.getTree();

    // Unknown zone names create a legacy top-level zone (backwards compatibility).
    tree->addCard("Legacy Card", 2, "custom_legacy_zone", -1);

    auto pairs = collectBoardCardPairs(deck);
    EXPECT_TRUE(hasPair(pairs, "custom_legacy_zone", "Legacy Card"));
    EXPECT_EQ(deck.getCardList({}).count("Legacy Card"), 1);
}

// =====================================================================================================================
// Zone management
// =====================================================================================================================

TEST(DeckListZones, RenameCustomZone)
{
    DeckList deck;
    auto *tree = deck.getTree();

    ASSERT_NE(tree->addCustomZone(DECK_ZONE_MAIN, "Removal"), nullptr);
    tree->addCard("Lightning Bolt", 2, "Removal", -1);

    EXPECT_TRUE(tree->renameCustomZone("Removal", "Bolt Zone"));
    EXPECT_TRUE(hasPair(collectBoardCardPairs(deck), DECK_ZONE_MAIN, "Lightning Bolt"));
    EXPECT_EQ(tree->getCustomZones(DECK_ZONE_MAIN).size(), 1);
    EXPECT_EQ(tree->getCustomZones(DECK_ZONE_MAIN).first()->getName(), QString("Bolt Zone"));

    // Renaming to a taken name fails.
    ASSERT_NE(tree->addCustomZone(DECK_ZONE_MAIN, "Other"), nullptr);
    EXPECT_FALSE(tree->renameCustomZone("Bolt Zone", "Other"));
    // Renaming a nonexistent zone fails.
    EXPECT_FALSE(tree->renameCustomZone("Ghost Zone", "Whatever"));
}

TEST(DeckListZones, MoveCustomZoneMovesCards)
{
    DeckList deck;
    auto *tree = deck.getTree();

    ASSERT_NE(tree->addCustomZone(DECK_ZONE_MAIN, "Removal"), nullptr);
    tree->addCard("Lightning Bolt", 2, "Removal", -1);

    EXPECT_TRUE(tree->moveCustomZone("Removal", DECK_ZONE_SIDE));

    auto pairs = collectBoardCardPairs(deck);
    EXPECT_TRUE(hasPair(pairs, DECK_ZONE_SIDE, "Lightning Bolt"));
    EXPECT_FALSE(hasPair(pairs, DECK_ZONE_MAIN, "Lightning Bolt"));

    // The custom zone is now nested under side.
    EXPECT_EQ(tree->getCustomZones(DECK_ZONE_MAIN).size(), 0);
    EXPECT_EQ(tree->getCustomZones(DECK_ZONE_SIDE).size(), 1);

    // Moving to an unknown board fails.
    EXPECT_FALSE(tree->moveCustomZone("Removal", "not_a_board"));
}

TEST(DeckListZones, MoveCustomZoneFailsForUnknownBoard)
{
    DeckList deck;
    auto *tree = deck.getTree();

    ASSERT_NE(tree->addCustomZone(DECK_ZONE_MAIN, "Removal"), nullptr);
    tree->addCard("Lightning Bolt", 2, "Removal", -1);

    EXPECT_FALSE(tree->moveCustomZone("Removal", "not_a_board"));

    // The zone is still under main.
    EXPECT_EQ(tree->getCustomZones(DECK_ZONE_MAIN).size(), 1);
}

// Regression: findCustomZoneByName walks every top-level zone, so a custom zone
// an imported deck carries under a non-standard board (tokens) is still found and
// movable. The pre-fix manager-level moveCustomZone only scanned the standard
// boards and returned false for these with no feedback.
TEST(DeckListZones, MoveCustomZoneNestedUnderTokensBoard)
{
    DeckList deck;
    auto *tree = deck.getTree();
    auto *root = tree->getRoot();

    auto *tokens = new InnerDecklistNode(DECK_ZONE_TOKENS, root);
    auto *removal = new InnerDecklistNode("Removal", tokens);
    new DecklistCardNode("Lightning Bolt", 2, removal, -1);

    EXPECT_TRUE(tree->findCustomZoneByName("Removal"));
    EXPECT_TRUE(tree->moveCustomZone("Removal", DECK_ZONE_SIDE));

    auto pairs = collectBoardCardPairs(deck);
    EXPECT_FALSE(hasPair(pairs, DECK_ZONE_TOKENS, "Lightning Bolt"));
    EXPECT_TRUE(hasPair(pairs, DECK_ZONE_SIDE, "Lightning Bolt"));
}

TEST(DeckListZones, RemoveCustomZoneRemovesCards)
{
    DeckList deck;
    auto *tree = deck.getTree();

    ASSERT_NE(tree->addCustomZone(DECK_ZONE_MAIN, "Removal"), nullptr);
    tree->addCard("Lightning Bolt", 2, "Removal", -1);

    EXPECT_TRUE(tree->removeCustomZone("Removal"));
    EXPECT_EQ(tree->getCustomZones(DECK_ZONE_MAIN).size(), 0);
    EXPECT_TRUE(deck.getCardList({DECK_ZONE_MAIN}).isEmpty());
    EXPECT_FALSE(tree->removeCustomZone("Removal"));
}

TEST(DeckListZones, EmptyCustomZoneIsKeptOnCardDeletion)
{
    DeckList deck;
    auto *tree = deck.getTree();

    ASSERT_NE(tree->addCustomZone(DECK_ZONE_MAIN, "Removal"), nullptr);
    auto *card = tree->addCard("Lightning Bolt", 1, "Removal", -1);

    // Deleting the last card must not delete the empty custom zone.
    EXPECT_TRUE(tree->deleteNode(card));
    EXPECT_EQ(tree->getCustomZones(DECK_ZONE_MAIN).size(), 1);
}

// =====================================================================================================================
// Deck-wide behavior
// =====================================================================================================================

TEST(DeckListZones, HashCountsCustomZoneCardsByBoard)
{
    // Deck A: cards directly in main and side.
    DeckList direct;
    direct.addCard("Mountain", DECK_ZONE_MAIN);
    direct.addCard("Lightning Bolt", DECK_ZONE_MAIN);
    direct.addCard("Island", DECK_ZONE_SIDE);

    // Deck B: identical, but organized in custom zones.
    DeckList organized;
    auto *tree = organized.getTree();
    ASSERT_NE(tree->addCustomZone(DECK_ZONE_MAIN, "Lands"), nullptr);
    ASSERT_NE(tree->addCustomZone(DECK_ZONE_MAIN, "Removal"), nullptr);
    ASSERT_NE(tree->addCustomZone(DECK_ZONE_SIDE, "Side Tech"), nullptr);
    tree->addCard("Mountain", 1, "Lands", -1);
    tree->addCard("Lightning Bolt", 1, "Removal", -1);
    tree->addCard("Island", 1, "Side Tech", -1);

    EXPECT_EQ(direct.getDeckHash(), organized.getDeckHash());
}

TEST(DeckListZones, SideboardSizeCountsCustomZoneCards)
{
    DeckList deck;
    auto *tree = deck.getTree();

    ASSERT_NE(tree->addCustomZone(DECK_ZONE_SIDE, "Side Tech"), nullptr);
    tree->addCard("Island", 3, "Side Tech", -1);
    tree->addCard("Forest", 2, DECK_ZONE_SIDE, -1);

    EXPECT_EQ(deck.getSideboardSize(), 5);
}

TEST(DeckListZones, PlainExportIncludesMainAndSideCustomZones)
{
    DeckList deck;
    auto *tree = deck.getTree();

    ASSERT_NE(tree->addCustomZone(DECK_ZONE_MAIN, "Removal"), nullptr);
    ASSERT_NE(tree->addCustomZone(DECK_ZONE_SIDE, "Side Tech"), nullptr);
    tree->addCard("Lightning Bolt", 2, "Removal", -1);
    tree->addCard("Island", 1, "Side Tech", -1);

    const QString plain = deck.writeToString_Plain(false, false);
    EXPECT_TRUE(plain.contains("2 Lightning Bolt"));
    EXPECT_TRUE(plain.contains("1 Island"));
}

TEST(DeckListZones, PlainExportSkipsMaybeboardCustomZones)
{
    DeckList deck;
    auto *tree = deck.getTree();

    ASSERT_NE(tree->addCustomZone(DECK_ZONE_MAYBEBOARD, "Candidates"), nullptr);
    tree->addCard("Wish Card", 4, "Candidates", -1);
    tree->addCard("Mountain", 1, DECK_ZONE_MAIN, -1);

    const QString plain = deck.writeToString_Plain(false, false);
    EXPECT_FALSE(plain.contains("Wish Card"));
    EXPECT_TRUE(plain.contains("1 Mountain"));
}

TEST(DeckListZones, NativeRoundTripPreservesCustomZones)
{
    DeckList deck;
    auto *tree = deck.getTree();

    ASSERT_NE(tree->addCustomZone(DECK_ZONE_MAIN, "Removal"), nullptr);
    ASSERT_NE(tree->addCustomZone(DECK_ZONE_SIDE, "Side Tech"), nullptr);
    tree->addCard("Lightning Bolt", 2, "Removal", -1);
    tree->addCard("Island", 3, "Side Tech", -1);
    tree->addCard("Mountain", 1, DECK_ZONE_MAIN, -1);

    // Round-trip through the native format.
    DeckList restored(deck.writeToString_Native());
    auto *restoredTree = restored.getTree();

    EXPECT_EQ(restored.getDeckHash(), deck.getDeckHash());
    EXPECT_EQ(restoredTree->getCustomZones(DECK_ZONE_MAIN).size(), 1);
    EXPECT_EQ(restoredTree->getCustomZones(DECK_ZONE_MAIN).first()->getName(), QString("Removal"));
    EXPECT_EQ(restoredTree->getCustomZones(DECK_ZONE_SIDE).size(), 1);

    auto pairs = collectBoardCardPairs(restored);
    EXPECT_TRUE(hasPair(pairs, DECK_ZONE_MAIN, "Lightning Bolt"));
    EXPECT_TRUE(hasPair(pairs, DECK_ZONE_SIDE, "Island"));
    EXPECT_TRUE(hasPair(pairs, DECK_ZONE_MAIN, "Mountain"));
    EXPECT_EQ(totalCards(pairs), 6);
}

TEST(DeckListZones, MaybeboardCustomZoneCardsAreExcludedFromHash)
{
    // Maybeboard cards are editor-only and must never affect the deck hash.
    DeckList deck;
    auto *tree = deck.getTree();

    ASSERT_NE(tree->addCustomZone(DECK_ZONE_MAYBEBOARD, "Candidates"), nullptr);
    tree->addCard("Wish Card", 4, "Candidates", -1);
    tree->addCard("Mountain", 1, DECK_ZONE_MAIN, -1);

    DeckList expected;
    expected.addCard("Mountain", DECK_ZONE_MAIN);

    EXPECT_EQ(deck.getDeckHash(), expected.getDeckHash());
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
