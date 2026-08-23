#include "deck_list_model_custom_zones.h"

#include "deck_list_model.h"

#include <../../../../libcockatrice_deck_list/libcockatrice/deck_list/tree/deck_list_card_node.h>
#include <QHash>
#include <QVector>

namespace DeckListModelCustomZones
{

bool isCustomZone(const AbstractDecklistNode *node)
{
    return dynamic_cast<const DecklistModelSubZoneNode *>(node) != nullptr;
}

void mirrorCustomZones(const InnerDecklistNode *deckBoardZone, InnerDecklistNode *shadowBoardZone)
{
    for (int j = 0; j < deckBoardZone->size(); j++) {
        auto *customCard = dynamic_cast<DecklistCardNode *>(deckBoardZone->at(j));
        if (customCard) {
            continue;
        }

        auto *customZone = dynamic_cast<const InnerDecklistNode *>(deckBoardZone->at(j));
        if (!customZone) {
            continue;
        }

        auto *shadowZone = new DecklistModelSubZoneNode(customZone->getName(), shadowBoardZone);
        for (int k = 0; k < customZone->size(); k++) {
            if (auto *zoneCard = dynamic_cast<DecklistCardNode *>(customZone->at(k))) {
                new DecklistModelCardNode(zoneCard, shadowZone);
            }
        }
    }
}

DecklistModelSubZoneNode *findSubZoneByName(InnerDecklistNode *root, const QString &zoneName)
{
    for (int i = 0; i < root->size(); i++) {
        auto *boardZone = dynamic_cast<InnerDecklistNode *>(root->at(i));
        if (!boardZone) {
            continue;
        }

        for (int j = 0; j < boardZone->size(); j++) {
            auto *customZone = dynamic_cast<DecklistModelSubZoneNode *>(boardZone->at(j));
            if (customZone && customZone->getName() == zoneName) {
                return customZone;
            }
        }
    }

    return nullptr;
}

namespace
{

/**
 * @brief Sorts a node's children and returns the (preSortRow, finalRow) mapping.
 */
QList<QPair<int, int>> plainSort(InnerDecklistNode *node, Qt::SortOrder order)
{
    const QVector<QPair<int, int>> sortResult = node->sort(order);

    QList<QPair<int, int>> mapping;
    mapping.reserve(node->size());
    for (int i = 0; i < node->size(); ++i) {
        mapping.append({sortResult[i].first, i});
    }
    return mapping;
}

/**
 * @brief Sorts a board zone's children, then stably moves custom zones to the end.
 *
 * @return The (preSortRow, finalRow) mapping covering both the sort and the shift.
 */
QList<QPair<int, int>> boardSort(InnerDecklistNode *node, Qt::SortOrder order)
{
    const QVector<QPair<int, int>> sortResult = node->sort(order);

    QVector<AbstractDecklistNode *> groups;
    QVector<AbstractDecklistNode *> customZones;
    QHash<AbstractDecklistNode *, int> preSortRowOf;

    groups.reserve(node->size());
    customZones.reserve(node->size());

    for (int i = 0; i < node->size(); ++i) {
        AbstractDecklistNode *child = node->at(i);
        preSortRowOf.insert(child, sortResult[i].first);
        if (isCustomZone(child)) {
            customZones.append(child);
        } else {
            groups.append(child);
        }
    }

    QVector<AbstractDecklistNode *> ordered = groups + customZones;
    for (int i = 0; i < ordered.size(); ++i) {
        node->replace(i, ordered[i]);
    }

    QList<QPair<int, int>> mapping;
    mapping.reserve(ordered.size());
    for (int i = 0; i < ordered.size(); ++i) {
        mapping.append({preSortRowOf.value(ordered[i]), i});
    }
    return mapping;
}

} // namespace

QList<QPair<int, int>> sortWithCustomZonesLast(InnerDecklistNode *root, InnerDecklistNode *node, Qt::SortOrder order)
{
    const bool isBoardZone = (node != root) && (node->getParent() == root);
    return isBoardZone ? boardSort(node, order) : plainSort(node, order);
}

} // namespace DeckListModelCustomZones
