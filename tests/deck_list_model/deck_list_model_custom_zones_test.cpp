/**
 * @file deck_list_model_custom_zones_test.cpp
 * @brief Tests for the deck list model's custom-zone shadow-tree helpers.
 *
 * DeckListModelCustomZones centralizes every "what is / where is a custom zone"
 * decision for the model's shadow tree: type testing, mirroring from the deck
 * tree, name lookup, and the sort-with-custom-zones-last ordering. These tests
 * exercise that logic directly on hand-built shadow trees, independent of the
 * full model and card database machinery.
 */

#include <gtest/gtest.h>
#include <libcockatrice/deck_list/tree/deck_list_card_node.h>
#include <libcockatrice/deck_list/tree/inner_deck_list_node.h>
#include <libcockatrice/models/deck_list/deck_list_model.h>

namespace
{

DecklistModelCardNode *cardNode(InnerDecklistNode *parent, const QString &name, int number)
{
    // The underlying data node is detached; only the model wrapper is attached to the shadow tree.
    auto *data = new DecklistCardNode(name, number, nullptr);
    return new DecklistModelCardNode(data, parent);
}

QStringList childNames(const InnerDecklistNode *node)
{
    QStringList names;
    for (int i = 0; i < node->size(); ++i) {
        names.append(node->at(i)->getName());
    }
    return names;
}

} // namespace

// =====================================================================================================================
// isCustomZone
// =====================================================================================================================

TEST(DeckListModelCustomZones, IsCustomZoneDistinguishesZoneFromGroup)
{
    InnerDecklistNode root;
    auto *board = new InnerDecklistNode(DECK_ZONE_MAIN, &root);
    auto *group = new InnerDecklistNode("Creature", board);
    auto *zone = new DecklistModelSubZoneNode("Removal", board);

    auto *card = cardNode(group, "A", 1);

    EXPECT_FALSE(DeckListModelCustomZones::isCustomZone(board));
    EXPECT_FALSE(DeckListModelCustomZones::isCustomZone(group));
    EXPECT_FALSE(DeckListModelCustomZones::isCustomZone(card));
    EXPECT_TRUE(DeckListModelCustomZones::isCustomZone(zone));
}

// =====================================================================================================================
// findSubZoneByName
// =====================================================================================================================

TEST(DeckListModelCustomZones, FindSubZoneByNameFindsAcrossBoards)
{
    InnerDecklistNode root;
    auto *main = new InnerDecklistNode(DECK_ZONE_MAIN, &root);
    auto *side = new InnerDecklistNode(DECK_ZONE_SIDE, &root);
    new DecklistModelSubZoneNode("Removal", main);
    new DecklistModelSubZoneNode("Utility", side);
    new InnerDecklistNode("Plain", main); // not a custom zone

    auto *removal = DeckListModelCustomZones::findSubZoneByName(&root, "Removal");
    ASSERT_NE(removal, nullptr);
    EXPECT_EQ(removal->getName(), QString("Removal"));

    auto *utility = DeckListModelCustomZones::findSubZoneByName(&root, "Utility");
    ASSERT_NE(utility, nullptr);
    EXPECT_EQ(utility->getName(), QString("Utility"));

    // Names are deck-unique; a plain group or built-in board is not matched.
    EXPECT_EQ(DeckListModelCustomZones::findSubZoneByName(&root, "Plain"), nullptr);
    EXPECT_EQ(DeckListModelCustomZones::findSubZoneByName(&root, DECK_ZONE_MAIN), nullptr);
    EXPECT_EQ(DeckListModelCustomZones::findSubZoneByName(&root, "Missing"), nullptr);
}

// =====================================================================================================================
// mirrorCustomZones
// =====================================================================================================================

TEST(DeckListModelCustomZones, MirrorCustomZonesCopiesCardsFlat)
{
    // Deck-tree board zone: one direct card plus one nested custom zone.
    auto *deckBoard = new InnerDecklistNode(DECK_ZONE_MAIN);
    new DecklistCardNode("Direct", 2, deckBoard);

    auto *deckZone = new InnerDecklistNode("Removal", deckBoard);
    auto *deckCard1 = new DecklistCardNode("Bolt", 3, deckZone);
    auto *deckCard2 = new DecklistCardNode("Swords", 1, deckZone);

    InnerDecklistNode shadowRoot;
    auto *shadowBoard = new InnerDecklistNode(DECK_ZONE_MAIN, &shadowRoot);

    DeckListModelCustomZones::mirrorCustomZones(deckBoard, shadowBoard);

    // Only the custom zone is mirrored as a sub-zone; the direct card is not.
    ASSERT_EQ(shadowBoard->size(), 1);
    auto *shadowZone = dynamic_cast<DecklistModelSubZoneNode *>(shadowBoard->at(0));
    ASSERT_NE(shadowZone, nullptr);
    EXPECT_EQ(shadowZone->getName(), QString("Removal"));

    // Cards live flat (un-grouped) inside the mirrored zone, wrapping the same data nodes.
    ASSERT_EQ(shadowZone->size(), 2);
    auto *shadowCard1 = dynamic_cast<DecklistModelCardNode *>(shadowZone->at(0));
    auto *shadowCard2 = dynamic_cast<DecklistModelCardNode *>(shadowZone->at(1));
    ASSERT_NE(shadowCard1, nullptr);
    ASSERT_NE(shadowCard2, nullptr);
    EXPECT_EQ(shadowCard1->getDataNode(), deckCard1);
    EXPECT_EQ(shadowCard2->getDataNode(), deckCard2);
}

TEST(DeckListModelCustomZones, MirrorCustomZonesWithNoCustomZonesIsNoop)
{
    // A board zone with only direct cards has nothing to mirror.
    auto *deckBoard = new InnerDecklistNode(DECK_ZONE_MAIN);
    new DecklistCardNode("Direct", 2, deckBoard);

    InnerDecklistNode shadowRoot;
    auto *shadowBoard = new InnerDecklistNode(DECK_ZONE_MAIN, &shadowRoot);

    DeckListModelCustomZones::mirrorCustomZones(deckBoard, shadowBoard);
    EXPECT_EQ(shadowBoard->size(), 0);
}

// =====================================================================================================================
// sortWithCustomZonesLast
// =====================================================================================================================

TEST(DeckListModelCustomZones, SortBoardKeepsCustomZonesAfterGroupsAscending)
{
    InnerDecklistNode root;
    auto *board = new InnerDecklistNode(DECK_ZONE_MAIN, &root);
    new DecklistModelSubZoneNode("Zebra", board);
    new InnerDecklistNode("Creature", board);
    new InnerDecklistNode("Instant", board);
    new DecklistModelSubZoneNode("Alpha", board);

    root.setSortMethod(DeckSortMethod::ByName);

    auto mapping = DeckListModelCustomZones::sortWithCustomZonesLast(&root, board, Qt::AscendingOrder);

    // Groups sort first (by name), then custom zones (by name), always after groups.
    EXPECT_EQ(childNames(board), (QStringList{"Creature", "Instant", "Alpha", "Zebra"}));

    // Some non-identity movement occurred.
    EXPECT_FALSE(mapping.isEmpty());
}

TEST(DeckListModelCustomZones, SortBoardKeepsCustomZonesAfterGroupsDescending)
{
    InnerDecklistNode root;
    auto *board = new InnerDecklistNode(DECK_ZONE_MAIN, &root);
    new DecklistModelSubZoneNode("Zebra", board);
    new InnerDecklistNode("Creature", board);
    new InnerDecklistNode("Instant", board);
    new DecklistModelSubZoneNode("Alpha", board);

    root.setSortMethod(DeckSortMethod::ByName);

    (void)DeckListModelCustomZones::sortWithCustomZonesLast(&root, board, Qt::DescendingOrder);

    // Groups still lead (descending), custom zones still last.
    EXPECT_EQ(childNames(board), (QStringList{"Instant", "Creature", "Zebra", "Alpha"}));
}

TEST(DeckListModelCustomZones, SortBoardMappingIsConsistent)
{
    InnerDecklistNode root;
    auto *board = new InnerDecklistNode(DECK_ZONE_MAIN, &root);

    QList<AbstractDecklistNode *> originalOrder;
    auto *g0 = new InnerDecklistNode("Creature", board);
    originalOrder.append(g0);
    auto *z0 = new DecklistModelSubZoneNode("Zebra", board);
    originalOrder.append(z0);
    auto *g1 = new InnerDecklistNode("Instant", board);
    originalOrder.append(g1);
    auto *z1 = new DecklistModelSubZoneNode("Alpha", board);
    originalOrder.append(z1);

    root.setSortMethod(DeckSortMethod::ByName);

    auto mapping = DeckListModelCustomZones::sortWithCustomZonesLast(&root, board, Qt::AscendingOrder);

    // The mapping reports, for each final row, the original row of the node now sitting there.
    ASSERT_EQ(mapping.size(), board->size());
    for (const auto &move : mapping) {
        const int preSortRow = move.first;
        const int finalRow = move.second;
        ASSERT_GE(preSortRow, 0);
        ASSERT_LT(preSortRow, originalOrder.size());
        EXPECT_EQ(board->at(finalRow), originalOrder[preSortRow]) << "row " << finalRow;
    }

    // Final order sanity: groups first in name order, then custom zones.
    EXPECT_EQ(childNames(board), (QStringList{"Creature", "Instant", "Alpha", "Zebra"}));
}

TEST(DeckListModelCustomZones, SortPlainNodeDoesNotReorderCustomZones)
{
    // A non-board node (e.g. a group whose children are cards) is sorted plainly;
    // custom zones are not a special case there. Cards sort by name.
    InnerDecklistNode root;
    auto *board = new InnerDecklistNode(DECK_ZONE_MAIN, &root);
    auto *group = new InnerDecklistNode("Creature", board);
    cardNode(group, "Swords", 1);
    cardNode(group, "Bolt", 3);

    root.setSortMethod(DeckSortMethod::ByName);

    auto mapping = DeckListModelCustomZones::sortWithCustomZonesLast(&root, group, Qt::AscendingOrder);
    EXPECT_EQ(childNames(group), (QStringList{"Bolt", "Swords"}));
    ASSERT_EQ(mapping.size(), 2);
    EXPECT_EQ(mapping[0].first, 1); // "Bolt" was originally at row 1
    EXPECT_EQ(mapping[0].second, 0);
    EXPECT_EQ(mapping[1].first, 0);
    EXPECT_EQ(mapping[1].second, 1);
}
