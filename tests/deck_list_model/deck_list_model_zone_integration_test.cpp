#include <gtest/gtest.h>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/deck_list/deck_list_node_tree.h>
#include <libcockatrice/deck_list/tree/deck_list_card_node.h>
#include <libcockatrice/deck_list/tree/inner_deck_list_node.h>
#include <libcockatrice/models/deck_list/deck_list_model.h>

namespace
{

int totalCustomZoneRows(const DeckListModel &model)
{
    int count = 0;
    const int rootRows = model.rowCount(QModelIndex());
    for (int r = 0; r < rootRows; ++r) {
        const QModelIndex board = model.index(r, 0, QModelIndex());
        const int childRows = model.rowCount(board);
        for (int c = 0; c < childRows; ++c) {
            const QModelIndex child = model.index(c, 0, board);
            if (child.data(DeckRoles::IsCustomZoneRole).toBool()) {
                ++count;
            }
        }
    }
    return count;
}

} // namespace

// The "Add to Zone" combobox/submenu lists getCustomZoneNames(), which reads the
// deck tree. These verify the source data a freshly-created zone populates.

TEST(DeckListModelZoneIntegration, CreateZoneThenReadCustomZoneNames)
{
    QSharedPointer<DeckList> deck(new DeckList());
    DeckListModel model(nullptr, deck);
    auto *tree = deck->getTree();

    ASSERT_NE(tree->addCustomZone(DECK_ZONE_MAIN, "Removal"), nullptr);
    EXPECT_EQ(model.getCustomZoneNames(DECK_ZONE_MAIN), (QStringList{"Removal"}));
}

TEST(DeckListModelZoneIntegration, CreateTwoZonesThenReadBoth)
{
    QSharedPointer<DeckList> deck(new DeckList());
    DeckListModel model(nullptr, deck);
    auto *tree = deck->getTree();

    ASSERT_NE(tree->addCustomZone(DECK_ZONE_MAIN, "Removal"), nullptr);
    ASSERT_NE(tree->addCustomZone(DECK_ZONE_MAIN, "Utility"), nullptr);
    EXPECT_EQ(model.getCustomZoneNames(DECK_ZONE_MAIN), (QStringList{"Removal", "Utility"}));
}

// Mirroring regression: rebuildTree must mirror each custom zone exactly once.
TEST(DeckListModelZoneIntegration, RebuildTreeMirrorsEachZoneOnce)
{
    QSharedPointer<DeckList> deck(new DeckList());
    DeckListModel model(nullptr, deck);
    auto *tree = deck->getTree();

    // One direct mainboard card plus two nested custom zones.
    tree->addCard("Lightning Bolt", 2, DECK_ZONE_MAIN, -1);
    ASSERT_NE(tree->addCustomZone(DECK_ZONE_MAIN, "Removal"), nullptr);
    tree->addCard("Swords to Plowshares", 1, "Removal", -1);
    ASSERT_NE(tree->addCustomZone(DECK_ZONE_MAIN, "Utility"), nullptr);

    model.rebuildTree();

    EXPECT_EQ(model.getCustomZoneNames(DECK_ZONE_MAIN), (QStringList{"Removal", "Utility"}));
    EXPECT_EQ(totalCustomZoneRows(model), 2);
}
