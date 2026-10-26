/** @file deck_state_manager_test.cpp
 *  @brief Tests for DeckStateManager operations.
 *  @ingroup Tests
 */

#include "libcockatrice/deck_list/tree/inner_deck_list_node.h"

#include <deck_list_model.h>
#include <gtest/gtest.h>
#include <interface/widgets/deck_editor/deck_state_manager.h>
#include <libcockatrice/card/printing/exact_card.h>

namespace
{

/**
 * @brief Assert that the DeckStateManager's deck list contains only the given zone,
 * and that that zone only contains the given number of cards, all with the given card name.
 *
 * @param manager the DeckStateManager
 * @param restrictToZones In which zones to look for the zone and cards. Empty means use the entire deck
 * @param zone The only zone that should exist in the deck
 * @param cardName The name of the only card that should exist in the deck
 * @param cardNumber The number of cards that should be in the deck
 */
static void containsExactly(DeckStateManager &manager,
                            QSet<QString> restrictToZones,
                            QString zone,
                            QString cardName,
                            int cardNumber)
{
    QList<const InnerDecklistNode *> zones = manager.getDeckList().getZoneNodes(restrictToZones);
    ASSERT_EQ(zones.size(), 1);
    ASSERT_EQ(zones[0]->getName(), zone);
    ASSERT_EQ(zones[0]->size(), 1);
    const DecklistCardNode *card = dynamic_cast<const DecklistCardNode *>((*zones[0])[0]);
    ASSERT_NE(card, nullptr);
    ASSERT_EQ(card->getName(), cardName);
    ASSERT_EQ(card->getNumber(), cardNumber);
}

// =====================================================================================================================
// addCard
// =====================================================================================================================

TEST(DeckStateManager, AddCards)
{
    DeckStateManager manager;
    manager.addCard(ExactCard(CardInfo::newInstance("Lightning Bolt")), DECK_ZONE_MAIN);
    containsExactly(manager, {}, DECK_ZONE_MAIN, "Lightning Bolt", 1);
    manager.addCard(ExactCard(CardInfo::newInstance("Lightning Bolt")), DECK_ZONE_MAIN);
    containsExactly(manager, {}, DECK_ZONE_MAIN, "Lightning Bolt", 2);
}

TEST(DeckStateManager, AddCardZones)
{
    DeckStateManager manager;
    manager.addCard(ExactCard(CardInfo::newInstance("Grizzly Bears")), DECK_ZONE_MAIN);
    manager.addCard(ExactCard(CardInfo::newInstance("Lightning Bolt")), DECK_ZONE_SIDE);
    manager.addCard(ExactCard(CardInfo::newInstance("Swords to Plowshares")), DECK_ZONE_MAYBEBOARD);

    // Check if the cards got put into the correct zones.
    ASSERT_EQ(manager.getDeckList().getZoneNodes().size(), 3);
    containsExactly(manager, {DECK_ZONE_MAIN}, DECK_ZONE_MAIN, "Grizzly Bears", 1);
    containsExactly(manager, {DECK_ZONE_SIDE}, DECK_ZONE_SIDE, "Lightning Bolt", 1);
    containsExactly(manager, {DECK_ZONE_MAYBEBOARD}, DECK_ZONE_MAYBEBOARD, "Swords to Plowshares", 1);
}

TEST(DeckStateManager, AddTokens)
{
    DeckStateManager manager;
    manager.addCard(ExactCard(CardInfo::newInstance("Beast", "", true, {}, {}, {}, {}, {})), DECK_ZONE_MAIN);
    manager.addCard(ExactCard(CardInfo::newInstance("Beast", "", true, {}, {}, {}, {}, {})), DECK_ZONE_SIDE);
    manager.addCard(ExactCard(CardInfo::newInstance("Beast", "", true, {}, {}, {}, {}, {})), DECK_ZONE_MAYBEBOARD);
    manager.addCard(ExactCard(CardInfo::newInstance("Beast", "", true, {}, {}, {}, {}, {})), DECK_ZONE_TOKENS);

    // A token should always be put into the tokens zone.
    containsExactly(manager, {}, DECK_ZONE_TOKENS, "Beast", 4);
}

// =====================================================================================================================
// decrementCard
// =====================================================================================================================

TEST(DeckStateManager, DecrementCards)
{
    DeckStateManager manager;
    manager.addCard(ExactCard(CardInfo::newInstance("Lightning Bolt")), DECK_ZONE_MAIN);
    manager.addCard(ExactCard(CardInfo::newInstance("Lightning Bolt")), DECK_ZONE_MAIN);
    containsExactly(manager, {}, DECK_ZONE_MAIN, "Lightning Bolt", 2);

    // decrementCard uses the model to find the card, so we need to rebuild it.
    manager.getModel()->rebuildTree();

    // Removing from the sideboard should do nothing, as there is nothing in the side board.
    manager.decrementCard(ExactCard(CardInfo::newInstance("Lightning Bolt")), DECK_ZONE_SIDE);
    containsExactly(manager, {}, DECK_ZONE_MAIN, "Lightning Bolt", 2);

    // Remove the first card.
    manager.decrementCard(ExactCard(CardInfo::newInstance("Lightning Bolt")), DECK_ZONE_MAIN);
    containsExactly(manager, {}, DECK_ZONE_MAIN, "Lightning Bolt", 1);

    // Removing the second card should also remove the zone.
    manager.decrementCard(ExactCard(CardInfo::newInstance("Lightning Bolt")), DECK_ZONE_MAIN);
    ASSERT_EQ(manager.getDeckList().getZoneNodes().size(), 0);
}

TEST(DeckStateManager, DecrementTokens)
{
    DeckStateManager manager;
    manager.addCard(ExactCard(CardInfo::newInstance("Beast", "", true, {}, {}, {}, {}, {})), DECK_ZONE_MAIN);
    manager.addCard(ExactCard(CardInfo::newInstance("Beast", "", true, {}, {}, {}, {}, {})), DECK_ZONE_SIDE);
    manager.addCard(ExactCard(CardInfo::newInstance("Beast", "", true, {}, {}, {}, {}, {})), DECK_ZONE_TOKENS);
    containsExactly(manager, {}, DECK_ZONE_TOKENS, "Beast", 3);

    // decrementCard uses the model to find the card, so we need to rebuild it.
    manager.getModel()->rebuildTree();

    manager.decrementCard(ExactCard(CardInfo::newInstance("Beast", "", true, {}, {}, {}, {}, {})), DECK_ZONE_TOKENS);
    containsExactly(manager, {}, DECK_ZONE_TOKENS, "Beast", 2);

    // Removing tokens from a given zone should remove them from the
    // token zone instead if that zone does not contain the token.
    manager.decrementCard(ExactCard(CardInfo::newInstance("Beast", "", true, {}, {}, {}, {}, {})), DECK_ZONE_SIDE);
    containsExactly(manager, {}, DECK_ZONE_TOKENS, "Beast", 1);

    manager.decrementCard(ExactCard(CardInfo::newInstance("Beast", "", true, {}, {}, {}, {}, {})), DECK_ZONE_MAIN);
    ASSERT_EQ(manager.getDeckList().getZoneNodes().size(), 0);
}
} // namespace

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
