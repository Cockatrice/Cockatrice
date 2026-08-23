#ifndef DECK_LIST_MODEL_CUSTOM_ZONES_H
#define DECK_LIST_MODEL_CUSTOM_ZONES_H

#include <../../../../libcockatrice_deck_list/libcockatrice/deck_list/tree/inner_deck_list_node.h>
#include <QList>
#include <QPair>
#include <QtGlobal>

/**
 * @class DecklistModelSubZoneNode
 * @ingroup DeckModels
 * @brief Model node representing a custom zone nested under a board zone.
 *
 * Custom zones group cards by user-defined names (e.g. "Removal", "Utility")
 * inside a board zone. They are mirrored from the underlying deck tree so that
 * they can be told apart from criteria group nodes by type.
 */
class DecklistModelSubZoneNode : public InnerDecklistNode
{
public:
    using InnerDecklistNode::InnerDecklistNode;
};

/**
 * @namespace DeckListModelCustomZones
 * @ingroup DeckModels
 * @brief Tree-level helpers for the deck list model's custom-zone shadow nodes.
 *
 * The deck list model keeps a second "shadow" tree of InnerDecklistNode that
 * mirrors the canonical deck tree for grouping and sorting. Custom zones add a
 * layer of bookkeeping to that shadow tree: they must be mirrored alongside
 * criteria groups, always sort after the groups within a board, and be
 * resolvable by deck-unique name.
 *
 * This namespace centralizes every "what is / where is a custom zone" decision
 * so the model itself only wires the results into Qt model signals.
 */
namespace DeckListModelCustomZones
{

/**
 * @brief Whether the given node is a custom zone (as opposed to a criteria group).
 */
[[nodiscard]] bool isCustomZone(const AbstractDecklistNode *node);

/**
 * @brief Mirrors the custom zones of a deck board zone into its shadow board node.
 *
 * Each custom zone becomes a DecklistModelSubZoneNode under @p shadowBoardZone
 * with its cards as direct (un-grouped) children.
 *
 * @param deckBoardZone The board zone in the canonical deck tree.
 * @param shadowBoardZone The matching board zone in the model's shadow tree.
 */
void mirrorCustomZones(const InnerDecklistNode *deckBoardZone, InnerDecklistNode *shadowBoardZone);

/**
 * @brief Finds a custom zone in the shadow tree by deck-unique name.
 * @param root Root of the shadow tree.
 * @param zoneName The custom zone name to find.
 * @return The matching custom zone node, or nullptr if not found.
 */
[[nodiscard]] DecklistModelSubZoneNode *findSubZoneByName(InnerDecklistNode *root, const QString &zoneName);

/**
 * @brief Sorts a shadow node's children, keeping a board's custom zones last.
 *
 * Sorting alone would interleave custom zones with criteria groups by name, but
 * custom zones must always stay after the groups within a board, regardless of
 * name. This applies the sort and, for board zones, stably moves the custom
 * zones to the end.
 *
 * @param root Root of the shadow tree (used to classify board zones).
 * @param node The shadow node whose children are reordered.
 * @param order Sort order to apply.
 * @return A list of (preSortRow, finalRow) pairs describing how each node moved.
 */
[[nodiscard]] QList<QPair<int, int>>
sortWithCustomZonesLast(InnerDecklistNode *root, InnerDecklistNode *node, Qt::SortOrder order);

} // namespace DeckListModelCustomZones

#endif // DECK_LIST_MODEL_CUSTOM_ZONES_H
