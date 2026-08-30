#include <gtest/gtest.h>
#include <libcockatrice/card/card_info.h>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/card/game_specific_terms.h>
#include <libcockatrice/card/printing/exact_card.h>
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

QModelIndex findBoardIndex(const DeckListModel &model, const QString &boardName)
{
    for (int r = 0; r < model.rowCount(QModelIndex()); ++r) {
        const QModelIndex idx = model.index(r, 0, QModelIndex());
        if (idx.data(DeckRoles::IsCardRole).toBool()) {
            continue;
        }
        const QString name = idx.sibling(idx.row(), DeckListModelColumns::CARD_NAME).data(Qt::EditRole).toString();
        if (name == boardName) {
            return idx;
        }
    }
    return {};
}

QModelIndex findZoneRow(const DeckListModel &model, const QModelIndex &board)
{
    for (int r = 0; r < model.rowCount(board); ++r) {
        const QModelIndex child = model.index(r, 0, board);
        if (child.data(DeckRoles::IsCustomZoneRole).toBool()) {
            return child;
        }
    }
    return {};
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

// =====================================================================================================================
// Model behaviour: addCard routing, findCard lookup, removeRows guard, empty-zone survival.
// =====================================================================================================================

TEST(DeckListModelZoneIntegration, AddCardRoutesIntoMirroredCustomZone)
{
    QSharedPointer<DeckList> deck(new DeckList());
    DeckListModel model(nullptr, deck);
    auto *tree = deck->getTree();

    ASSERT_NE(tree->addCustomZone(DECK_ZONE_MAIN, "Removal"), nullptr);
    model.rebuildTree();

    QModelIndex added = model.addCard(ExactCard(CardInfo::newInstance("Lightning Bolt")), "Removal");
    ASSERT_TRUE(added.isValid());

    // The card is a direct child of the mirrored custom zone, not a new top-level zone.
    const QModelIndex zoneParent = added.parent();
    ASSERT_TRUE(zoneParent.isValid());
    EXPECT_TRUE(zoneParent.data(DeckRoles::IsCustomZoneRole).toBool());
    EXPECT_EQ(zoneParent.sibling(zoneParent.row(), DeckListModelColumns::CARD_NAME).data(Qt::DisplayRole).toString(),
              QString("Removal"));

    // No "Removal" top-level zone appeared in the deck tree.
    auto *listRoot = tree->getRoot();
    bool topLevelRemoval = false;
    for (int i = 0; i < listRoot->size(); ++i) {
        if (auto *zone = dynamic_cast<InnerDecklistNode *>(listRoot->at(i))) {
            topLevelRemoval |= zone->getName() == "Removal";
        }
    }
    EXPECT_FALSE(topLevelRemoval);
}

TEST(DeckListModelZoneIntegration, AddCardToUnmirroredCustomZoneRebuildsNotCreatesTopLevel)
{
    QSharedPointer<DeckList> deck(new DeckList());
    DeckListModel model(nullptr, deck);
    auto *tree = deck->getTree();

    // The zone exists on the deck tree but the shadow tree has never mirrored it.
    ASSERT_NE(tree->addCustomZone(DECK_ZONE_MAIN, "Removal"), nullptr);

    QModelIndex added = model.addCard(ExactCard(CardInfo::newInstance("Lightning Bolt")), "Removal");
    ASSERT_TRUE(added.isValid());

    const QModelIndex zoneParent = added.parent();
    ASSERT_TRUE(zoneParent.isValid());
    EXPECT_TRUE(zoneParent.data(DeckRoles::IsCustomZoneRole).toBool());
    EXPECT_EQ(zoneParent.sibling(zoneParent.row(), DeckListModelColumns::CARD_NAME).data(Qt::DisplayRole).toString(),
              QString("Removal"));
}

TEST(DeckListModelZoneIntegration, AddCardCreatesGroupSeparatelyFromSameNamedZone)
{
    QSharedPointer<DeckList> deck(new DeckList());
    DeckListModel model(nullptr, deck);
    auto *tree = deck->getTree();

    // A custom zone named exactly like a grouping criterion.
    ASSERT_NE(tree->addCustomZone(DECK_ZONE_MAIN, "Creature"), nullptr);
    model.rebuildTree();

    CardInfoPtr bear = CardInfo::newInstance("Grizzly Bears");
    bear->setProperty(Mtg::MainCardType, "Creature");

    QModelIndex added = model.addCard(ExactCard(bear), DECK_ZONE_MAIN);
    ASSERT_TRUE(added.isValid());

    // The card lands in a *group* node called "Creature", not swallowed by the custom zone.
    const QModelIndex groupParent = added.parent();
    ASSERT_TRUE(groupParent.isValid());
    EXPECT_FALSE(groupParent.data(DeckRoles::IsCustomZoneRole).toBool());
    EXPECT_EQ(groupParent.sibling(groupParent.row(), DeckListModelColumns::CARD_NAME).data(Qt::DisplayRole).toString(),
              QString("Creature"));

    // The board keeps both rows: the "Creature" group and the "Creature" custom zone.
    const QModelIndex boardIndex = groupParent.parent();
    ASSERT_TRUE(boardIndex.isValid());
    EXPECT_EQ(model.rowCount(boardIndex), 2);
}

TEST(DeckListModelZoneIntegration, FindCardResolvesCardInsideCustomZone)
{
    QSharedPointer<DeckList> deck(new DeckList());
    DeckListModel model(nullptr, deck);
    auto *tree = deck->getTree();

    ASSERT_NE(tree->addCustomZone(DECK_ZONE_MAIN, "Removal"), nullptr);
    model.rebuildTree();

    // findCard resolves through the card database; register the card we add.
    const QString cardName = "Swords to Plowshares";
    CardInfoPtr info = CardInfo::newInstance(cardName);
    CardDatabaseManager::getInstance()->addCard(info);

    QModelIndex added = model.addCard(ExactCard(info), "Removal");
    ASSERT_TRUE(added.isValid());

    QModelIndex found = model.findCard(cardName, "Removal");
    EXPECT_TRUE(found.isValid());
    EXPECT_EQ(found, added);
}

TEST(DeckListModelZoneIntegration, RemoveRowsRefusesCustomZoneRow)
{
    QSharedPointer<DeckList> deck(new DeckList());
    DeckListModel model(nullptr, deck);
    auto *tree = deck->getTree();

    ASSERT_NE(tree->addCustomZone(DECK_ZONE_MAIN, "Removal"), nullptr);
    tree->addCard("Lightning Bolt", 2, DECK_ZONE_MAIN, -1);
    model.rebuildTree();

    const QModelIndex mainIndex = findBoardIndex(model, DECK_ZONE_MAIN);
    ASSERT_TRUE(mainIndex.isValid());
    const QModelIndex zoneRow = findZoneRow(model, mainIndex);
    ASSERT_TRUE(zoneRow.isValid());

    EXPECT_FALSE(model.removeRow(zoneRow.row(), zoneRow.parent()));
    EXPECT_EQ(model.rowCount(mainIndex), 2); // the zone survives, alongside the card group
}

TEST(DeckListModelZoneIntegration, EmptyCustomZoneSurvivesMirrorAndPruning)
{
    QSharedPointer<DeckList> deck(new DeckList());
    DeckListModel model(nullptr, deck);
    auto *tree = deck->getTree();

    // An empty custom zone must be mirrored (the stack deliberately keeps it alive).
    ASSERT_NE(tree->addCustomZone(DECK_ZONE_MAIN, "Removal"), nullptr);
    model.rebuildTree();

    const QModelIndex mainIndex = findBoardIndex(model, DECK_ZONE_MAIN);
    ASSERT_TRUE(mainIndex.isValid());
    EXPECT_EQ(model.rowCount(mainIndex), 1);
    EXPECT_TRUE(findZoneRow(model, mainIndex).isValid());
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
