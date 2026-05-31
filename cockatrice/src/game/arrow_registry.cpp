#include "arrow_registry.h"

#include "board/arrow_item.h"

void ArrowRegistry::insert(QSharedPointer<ArrowData> data, ArrowItem *arrow)
{
    const ArrowKey key{data->creatorId, data->id};

    if (auto *existing = take(data->creatorId, data->id)) {
        existing->delArrow();
    }

    dataStore.insert(key, data);
    items.insert(key, arrow);
    byPlayer[data->creatorId].insert(data->id);
}

ArrowItem *ArrowRegistry::take(int creatorId, int arrowId)
{
    const ArrowKey key{creatorId, arrowId};
    dataStore.remove(key);
    auto &playerSet = byPlayer[creatorId];
    playerSet.remove(arrowId);
    if (playerSet.isEmpty()) {
        byPlayer.remove(creatorId);
    }
    return items.take(key);
}

ArrowItem *ArrowRegistry::get(int creatorId, int arrowId) const
{
    return items.value(ArrowKey{creatorId, arrowId}, nullptr);
}

bool ArrowRegistry::contains(int creatorId, int arrowId) const
{
    return items.contains(ArrowKey{creatorId, arrowId});
}

QSet<int> ArrowRegistry::idsForPlayer(int playerId) const
{
    return byPlayer.value(playerId);
}

QList<ArrowItem *> ArrowRegistry::all() const
{
    return items.values();
}