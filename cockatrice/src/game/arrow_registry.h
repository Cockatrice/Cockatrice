#ifndef COCKATRICE_ARROW_REGISTRY_H
#define COCKATRICE_ARROW_REGISTRY_H

#include "board/arrow_data.h"

#include <QMap>
#include <QSet>
#include <QSharedPointer>

class ArrowItem;

struct ArrowKey
{
    int creatorId;
    int arrowId;

    bool operator<(const ArrowKey &other) const
    {
        if (creatorId != other.creatorId) {
            return creatorId < other.creatorId;
        }
        return arrowId < other.arrowId;
    }
};

class ArrowRegistry
{
public:
    void insert(QSharedPointer<ArrowData> data, ArrowItem *arrow);
    ArrowItem *take(int creatorId, int arrowId);

    [[nodiscard]] ArrowItem *get(int creatorId, int arrowId) const;
    [[nodiscard]] bool contains(int creatorId, int arrowId) const;
    [[nodiscard]] QSet<int> idsForPlayer(int playerId) const;
    [[nodiscard]] QList<ArrowItem *> all() const;

private:
    QMap<ArrowKey, QSharedPointer<ArrowData>> dataStore;
    QMap<ArrowKey, ArrowItem *> items;
    QMap<int, QSet<int>> byPlayer;
};

#endif