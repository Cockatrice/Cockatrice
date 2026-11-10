#include "gtest/gtest.h"
#include <QList>
#include <QString>
#include <libcockatrice/deck_list/deck_diff.h>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/deck_list/tree/deck_list_card_node.h>
#include <libcockatrice/deck_list/tree/inner_deck_list_node.h>

namespace
{
DeckList makeDeck(std::initializer_list<std::pair<const char *, int>> mainCards,
                  std::initializer_list<std::pair<const char *, int>> sideCards = {})
{
    DeckList deck;
    for (const auto &[name, count] : mainCards) {
        for (int i = 0; i < count; ++i) {
            deck.addCard(name, "main");
        }
    }
    for (const auto &[name, count] : sideCards) {
        for (int i = 0; i < count; ++i) {
            deck.addCard(name, "side");
        }
    }
    return deck;
}

int countOf(const DeckList &deck, const char *zone, const char *name)
{
    int count = 0;
    deck.forEachCard([&](InnerDecklistNode *zoneNode, DecklistCardNode *card) {
        if (zoneNode->getName() == zone && card->getName() == name) {
            count += card->getNumber();
        }
    });
    return count;
}

QList<DecklistCardNode *> nodesOf(const DeckList &deck, const char *zone, const char *name)
{
    QList<DecklistCardNode *> nodes;
    deck.forEachCard([&](InnerDecklistNode *zoneNode, DecklistCardNode *card) {
        if (zoneNode->getName() == zone && card->getName() == name) {
            nodes.append(card);
        }
    });
    return nodes;
}
} // namespace

TEST(DeckDiffTest, IdenticalDecksHaveEmptyDiff)
{
    DeckList first = makeDeck({{"Lightning Bolt", 4}});
    DeckList second = makeDeck({{"Lightning Bolt", 4}});
    EXPECT_TRUE(DeckDiff::diff(first, second).isEmpty());
}

TEST(DeckDiffTest, ChangesAreSortedByZoneThenName)
{
    DeckList oldDeck = makeDeck({{"Mountain", 20}, {"Lightning Bolt", 4}}, {{"Shock", 2}});
    DeckList newDeck = makeDeck({{"Mountain", 19}, {"Shock", 5}}, {{"Shock", 1}, {"Teferi", 1}});

    DeckDiffResult result = DeckDiff::diff(oldDeck, newDeck);
    ASSERT_EQ(result.changes.size(), 5);
    EXPECT_EQ(result.changes.at(0).zone, "main");
    EXPECT_EQ(result.changes.at(0).cardName, "Lightning Bolt");
    EXPECT_EQ(result.changes.at(1).cardName, "Mountain");
    EXPECT_EQ(result.changes.at(2).cardName, "Shock");
    EXPECT_EQ(result.changes.at(3).zone, "side");
    EXPECT_EQ(result.changes.at(3).cardName, "Shock");
    EXPECT_EQ(result.changes.at(4).cardName, "Teferi");
}

TEST(DeckDiffTest, ReportsAddedAndRemovedCopies)
{
    DeckList oldDeck = makeDeck({{"Lightning Bolt", 4}, {"Mountain", 20}});
    DeckList newDeck = makeDeck({{"Lightning Bolt", 2}, {"Shock", 4}, {"Mountain", 20}});

    DeckDiffResult result = DeckDiff::diff(oldDeck, newDeck);
    ASSERT_EQ(result.changes.size(), 2);
    EXPECT_EQ(result.changes.at(0).cardName, "Lightning Bolt");
    EXPECT_EQ(result.changes.at(0).oldCount, 4);
    EXPECT_EQ(result.changes.at(0).newCount, 2);
    EXPECT_EQ(result.changes.at(0).delta(), -2);
    EXPECT_EQ(result.changes.at(1).cardName, "Shock");
    EXPECT_EQ(result.changes.at(1).oldCount, 0);
    EXPECT_EQ(result.changes.at(1).newCount, 4);
}

TEST(DeckDiffTest, ZoneMoveIsTwoChanges)
{
    DeckList oldDeck = makeDeck({{"Teferi", 1}});
    DeckList newDeck = makeDeck({}, {{"Teferi", 1}});

    DeckDiffResult result = DeckDiff::diff(oldDeck, newDeck);
    ASSERT_EQ(result.changes.size(), 2);
    EXPECT_EQ(result.changes.at(0).zone, "main");
    EXPECT_EQ(result.changes.at(0).newCount, 0);
    EXPECT_EQ(result.changes.at(1).zone, "side");
    EXPECT_EQ(result.changes.at(1).oldCount, 0);
}

TEST(DeckDiffTest, DisjointChangesCanAutoMerge)
{
    DeckList base = makeDeck({{"Lightning Bolt", 4}, {"Mountain", 20}});
    DeckList local = makeDeck({{"Lightning Bolt", 4}, {"Mountain", 18}});
    DeckList remote = makeDeck({{"Shock", 2}, {"Lightning Bolt", 4}, {"Mountain", 20}});

    DeckDiffResult localChanges = DeckDiff::diff(base, local);
    DeckDiffResult remoteChanges = DeckDiff::diff(base, remote);
    EXPECT_TRUE(DeckDiff::canAutoMerge(localChanges, remoteChanges));

    DeckList merged = DeckDiff::merge(base, localChanges, remoteChanges);
    EXPECT_EQ(countOf(merged, "main", "Mountain"), 18);
    EXPECT_EQ(countOf(merged, "main", "Shock"), 2);
    EXPECT_EQ(countOf(merged, "main", "Lightning Bolt"), 4);
}

TEST(DeckDiffTest, OverlappingChangesCannotAutoMergeAndRemoteWins)
{
    DeckList base = makeDeck({{"Lightning Bolt", 4}});
    DeckList local = makeDeck({{"Lightning Bolt", 3}});
    DeckList remote = makeDeck({{"Lightning Bolt", 1}});

    DeckDiffResult localChanges = DeckDiff::diff(base, local);
    DeckDiffResult remoteChanges = DeckDiff::diff(base, remote);
    EXPECT_FALSE(DeckDiff::canAutoMerge(localChanges, remoteChanges));

    DeckList merged = DeckDiff::merge(base, localChanges, remoteChanges);
    EXPECT_EQ(countOf(merged, "main", "Lightning Bolt"), 1);
}

TEST(DeckDiffTest, MergeKeepsBaseMetadata)
{
    DeckList base = makeDeck({{"Lightning Bolt", 4}});
    base.setName("Affinity v3");
    DeckList remote = makeDeck({{"Lightning Bolt", 4}, {"Shock", 1}});

    DeckList merged = DeckDiff::merge(base, DeckDiff::diff(base, base), DeckDiff::diff(base, remote));
    EXPECT_EQ(merged.getName(), "Affinity v3");
    EXPECT_EQ(countOf(merged, "main", "Shock"), 1);
}

TEST(DeckDiffTest, MergePreservesBasePrintingAndCollapsesNodes)
{
    DeckList base;
    base.addCard("Mountain", "main", -1, "DOM", "250")->setNumber(4);
    base.refreshDeckHash();

    DeckList local = base;
    local.addCard("Shock", "main", -1, "M19", "156")->setNumber(2);
    local.refreshDeckHash();

    DeckList remote = base;
    remote.addCard("Lightning Bolt", "main", -1, "M10", "146")->setNumber(4);
    remote.refreshDeckHash();

    const DeckDiffResult localChanges = DeckDiff::diff(base, local);
    const DeckDiffResult remoteChanges = DeckDiff::diff(base, remote);
    ASSERT_TRUE(DeckDiff::canAutoMerge(localChanges, remoteChanges));

    const DeckList merged = DeckDiff::merge(base, localChanges, remoteChanges);

    const QList<DecklistCardNode *> mountains = nodesOf(merged, "main", "Mountain");
    ASSERT_EQ(mountains.size(), 1);
    EXPECT_EQ(mountains.first()->getNumber(), 4);
    EXPECT_EQ(mountains.first()->getCardSetShortName(), "DOM");
    EXPECT_EQ(mountains.first()->getCardCollectorNumber(), "250");

    const QList<DecklistCardNode *> shocks = nodesOf(merged, "main", "Shock");
    ASSERT_EQ(shocks.size(), 1);
    EXPECT_EQ(shocks.first()->getNumber(), 2);
    EXPECT_EQ(shocks.first()->getCardSetShortName(), "M19");
    EXPECT_EQ(shocks.first()->getCardCollectorNumber(), "156");
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
