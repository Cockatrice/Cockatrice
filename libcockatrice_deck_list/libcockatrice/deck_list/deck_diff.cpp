#include "deck_diff.h"

#include "deck_list.h"
#include "tree/deck_list_card_node.h"
#include "tree/inner_deck_list_node.h"

#include <QMap>
#include <QPair>
#include <QSet>
#include <algorithm>

namespace
{
using ZoneCardKey = QPair<QString, QString>;

QMap<ZoneCardKey, int> zoneCardCounts(const DeckList &deck)
{
    QMap<ZoneCardKey, int> counts;
    deck.forEachCard([&counts](InnerDecklistNode *zone, DecklistCardNode *card) {
        counts[{zone->getName(), card->getName()}] += card->getNumber();
    });
    return counts;
}

bool changeLessThan(const DeckCardChange &a, const DeckCardChange &b)
{
    if (a.zone != b.zone) {
        return a.zone < b.zone;
    }
    return a.cardName < b.cardName;
}
} // namespace

DeckDiffResult DeckDiff::diff(const DeckList &oldDeck, const DeckList &newDeck)
{
    const QMap<ZoneCardKey, int> oldCounts = zoneCardCounts(oldDeck);
    const QMap<ZoneCardKey, int> newCounts = zoneCardCounts(newDeck);

    QSet<ZoneCardKey> keys(oldCounts.keyBegin(), oldCounts.keyEnd());
    keys.unite(QSet<ZoneCardKey>(newCounts.keyBegin(), newCounts.keyEnd()));

    DeckDiffResult result;
    for (const ZoneCardKey &key : keys) {
        const int oldCount = oldCounts.value(key, 0);
        const int newCount = newCounts.value(key, 0);
        if (oldCount != newCount) {
            result.changes.append({key.first, key.second, oldCount, newCount});
        }
    }
    std::sort(result.changes.begin(), result.changes.end(), changeLessThan);
    return result;
}

bool DeckDiff::canAutoMerge(const DeckDiffResult &localChanges, const DeckDiffResult &remoteChanges)
{
    QSet<ZoneCardKey> localKeys;
    for (const DeckCardChange &change : localChanges.changes) {
        localKeys.insert({change.zone, change.cardName});
    }
    for (const DeckCardChange &change : remoteChanges.changes) {
        if (localKeys.contains({change.zone, change.cardName})) {
            return false;
        }
    }
    return true;
}

DeckList
DeckDiff::merge(const DeckList &baseDeck, const DeckDiffResult &localChanges, const DeckDiffResult &remoteChanges)
{
    QMap<ZoneCardKey, int> counts = zoneCardCounts(baseDeck);
    for (const DeckCardChange &change : localChanges.changes) {
        counts[{change.zone, change.cardName}] = change.newCount;
    }
    for (const DeckCardChange &change : remoteChanges.changes) {
        counts[{change.zone, change.cardName}] = change.newCount;
    }

    DeckList merged(baseDeck.getMetadata(), DecklistNodeTree(), baseDeck.getSideboardPlans());
    QList<ZoneCardKey> keys = counts.keys();
    std::sort(keys.begin(), keys.end());
    for (const ZoneCardKey &key : keys) {
        const int count = qMax(0, counts.value(key));
        for (int i = 0; i < count; ++i) {
            merged.addCard(key.second, key.first);
        }
    }
    return merged;
}
