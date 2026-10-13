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

struct ZoneCardInfo
{
    int count = 0;
    QString cardSetShortName;
    QString cardSetNumber;
    QString cardProviderId;
};

QMap<ZoneCardKey, ZoneCardInfo> zoneCardInfos(const DeckList &deck)
{
    QMap<ZoneCardKey, ZoneCardInfo> infos;
    deck.forEachCard([&infos](InnerDecklistNode *zone, DecklistCardNode *card) {
        ZoneCardInfo &info = infos[{zone->getName(), card->getName()}];
        if (info.count == 0) {
            info.cardSetShortName = card->getCardSetShortName();
            info.cardSetNumber = card->getCardCollectorNumber();
            info.cardProviderId = card->getCardProviderId();
        }
        info.count += card->getNumber();
    });
    return infos;
}

QMap<ZoneCardKey, int> zoneCardCounts(const DeckList &deck)
{
    QMap<ZoneCardKey, int> counts;
    const QMap<ZoneCardKey, ZoneCardInfo> infos = zoneCardInfos(deck);
    for (auto it = infos.begin(); it != infos.end(); ++it) {
        counts.insert(it.key(), it.value().count);
    }
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
    const QMap<ZoneCardKey, ZoneCardInfo> newInfos = zoneCardInfos(newDeck);

    QSet<ZoneCardKey> keys(oldCounts.keyBegin(), oldCounts.keyEnd());
    keys.unite(QSet<ZoneCardKey>(newInfos.keyBegin(), newInfos.keyEnd()));

    DeckDiffResult result;
    for (const ZoneCardKey &key : keys) {
        const int oldCount = oldCounts.value(key, 0);
        const ZoneCardInfo newInfo = newInfos.value(key, ZoneCardInfo());
        if (oldCount != newInfo.count) {
            DeckCardChange change;
            change.zone = key.first;
            change.cardName = key.second;
            change.oldCount = oldCount;
            change.newCount = newInfo.count;
            change.cardSetShortName = newInfo.cardSetShortName;
            change.cardSetNumber = newInfo.cardSetNumber;
            change.cardProviderId = newInfo.cardProviderId;
            result.changes.append(change);
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
    QMap<ZoneCardKey, DeckCardChange> printings;
    for (const DeckCardChange &change : localChanges.changes) {
        counts[{change.zone, change.cardName}] = change.newCount;
        printings[{change.zone, change.cardName}] = change;
    }
    for (const DeckCardChange &change : remoteChanges.changes) {
        counts[{change.zone, change.cardName}] = change.newCount;
        printings[{change.zone, change.cardName}] = change;
    }

    DeckList merged = baseDeck;
    QMap<ZoneCardKey, QList<DecklistCardNode *>> nodesByKey;
    merged.forEachCard([&nodesByKey](InnerDecklistNode *zone, DecklistCardNode *card) {
        nodesByKey[{zone->getName(), card->getName()}].append(card);
    });

    for (auto it = counts.begin(); it != counts.end(); ++it) {
        const ZoneCardKey &key = it.key();
        const int target = qMax(0, it.value());
        QList<DecklistCardNode *> nodes = nodesByKey.value(key);
        if (target == 0) {
            for (DecklistCardNode *node : nodes) {
                merged.getTree()->deleteNode(node);
            }
        } else if (nodes.isEmpty()) {
            const DeckCardChange printing = printings.value(key);
            merged.getTree()->addCard(printing.cardName, target, printing.zone, -1, printing.cardSetShortName,
                                      printing.cardSetNumber, printing.cardProviderId, true);
        } else {
            nodes.first()->setNumber(target);
            for (int i = 1; i < nodes.size(); ++i) {
                merged.getTree()->deleteNode(nodes.at(i));
            }
        }
    }
    merged.refreshDeckHash();
    return merged;
}
