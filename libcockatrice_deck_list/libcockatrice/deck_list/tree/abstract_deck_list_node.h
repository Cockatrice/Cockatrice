/**
 * @file abstract_deck_list_node.h
 * @brief Defines the AbstractDecklistNode base class used as the foundation
 *        for all nodes in the deck list tree (zones, groups, and cards).
 *
 * The deck list is modeled as a tree:
 * - The invisible root node is managed by DeckListModel.
 * - Top-level children are zones (e.g. Mainboard, Sideboard).
 * - Zones contain grouping nodes (e.g. by type, color, or mana cost).
 * - Grouping nodes contain card nodes.
 *
 * This abstract base class provides the interface and shared functionality
 * for all node types. Concrete subclasses (InnerDecklistNode,
 * DecklistCardNode, DecklistModelCardNode, etc.) implement the specifics.
 */

#ifndef COCKATRICE_ABSTRACT_DECK_LIST_NODE_H
#define COCKATRICE_ABSTRACT_DECK_LIST_NODE_H

#include <QtCore/QXmlStreamWriter>

/**
 * @enum DeckSortMethod
 * @ingroup DeckModels
 * @brief Defines the different sort strategies a node may use
 *        to order its children.
 *
 * Sorting behavior is typically set by the DeckListModel when the user
 * requests sorting in the UI.
 *
 * - ByNumber: Sort numerically (often by collector number).
 * - ByName:   Sort alphabetically by card name.
 * - Default:  No explicit sorting; insertion order is preserved.
 */
enum DeckSortMethod
{
    ByNumber, ///< Sort by numeric properties (e.g. collector number).
    ByName,   ///< Sort by card name (locale-aware comparison).
    Default   ///< Leave in insertion order.
};

class InnerDecklistNode;

/**
 * @class AbstractDecklistNode
 * @ingroup DeckModels
 * @brief Base class for all nodes in the deck list tree.
 *
 * This class defines the common interface for every node in the
 * deck representation: zones, groupings, and cards.
 *
 * Responsibilities:
 * - Maintain a pointer to its parent (if any).
 * - Track the sorting method to be used for child nodes.
 * - Provide a consistent interface for retrieving basic identifying
 *   properties (name, set, collector number, provider ID).
 * - Define abstract methods for XML serialization, used when saving
 *   or loading deck files.
 *
 * Lifetime / Ownership:
 * - Nodes are arranged hierarchically under @c InnerDecklistNode parents.
 * - The parent takes ownership of its children; destruction cascades.
 * - The DeckListModel holds the invisible root node, which in turn
 *   owns the entire hierarchy.
 *
 * Extension:
 * - @c InnerDecklistNode is the concrete subclass representing
 *   "folders" in the tree (zones, groups).
 * - @c DecklistCardNode and @c DecklistModelCardNode represent
 *   actual card entries.
 */
class AbstractDecklistNode
{
protected:
    /**
     * @brief Pointer to the parent node, or nullptr if this is the root.
     *
     * Ownership note: The parent is responsible for destroying this node
     * when it is removed from the tree.
     */
    InnerDecklistNode *parent;

    /**
     * @brief Current sorting strategy for this node's children.
     *
     * Sorting is applied recursively by the DeckListModel when
     * the view requests it.
     */
    DeckSortMethod sortMethod;

public:
    /**
     * @brief Construct a new AbstractDecklistNode and insert it into its parent.
     *
     * @param _parent Parent node. May be nullptr if this is the root.
     * @param position Optional index at which to insert into the parent's
     *        children. If -1, the node is appended to the end.
     *
     * If a parent is provided, the constructor automatically appends
     * or inserts this node into the parent’s child list.
     */
    explicit AbstractDecklistNode(InnerDecklistNode *_parent = nullptr, int position = -1);

    /// Virtual destructor. Child classes must clean up their resources.
    virtual ~AbstractDecklistNode() = default;

    /**
     * @brief Set the sort method for this node’s children.
     * @param method The sorting strategy to use.
     *
     * Subclasses may override if they need to apply additional logic.
     */
    virtual void setSortMethod(DeckSortMethod method)
    {
        sortMethod = method;
    }

    /**
     * @name Core identification properties
     *
     * These methods provide a standard way for the model to retrieve
     * identifying information about a node, regardless of type.
     * @{
     */
    [[nodiscard]] virtual QString getName() const = 0;
    [[nodiscard]] virtual QString getCardProviderId() const = 0;
    [[nodiscard]] virtual QString getCardSetShortName() const = 0;
    [[nodiscard]] virtual QString getCardCollectorNumber() const = 0;
    /// @}

    /**
     * @brief Whether this node is the "deck header" (deck metadata).
     *
     * This distinguishes special nodes that represent deck-level
     * information rather than cards or groupings.
     */
    [[nodiscard]] virtual bool isDeckHeader() const = 0;

    /// @return The parent node, or nullptr if this is the root.
    [[nodiscard]] InnerDecklistNode *getParent() const
    {
        return parent;
    }

    /**
     * @brief Compute the depth of this node in the tree.
     * @return Distance from the root (root = 0, children = 1, etc.).
     */
    [[nodiscard]] int depth() const;

    /**
     * @brief Compute the "height" of this node.
     *
     * Height is defined by subclasses; it usually represents how
     * many levels of descendants this node spans.
     *
     * For example:
     * - A card node has height 1.
     * - A group node containing cards has height 2.
     */
    [[nodiscard]] virtual int height() const = 0;

    /**
     * @brief Compare this node against another for sorting.
     *
     * The semantics of comparison depend on the node type and the
     * current @c sortMethod.
     *
     * @param other The node to compare against.
     * @return true if this node should come before @p other.
     */
    virtual bool compare(AbstractDecklistNode *other) const = 0;

    /**
     * @name XML serialization
     * These methods support reading and writing decks from/to
     * Cockatrice deck XML format.
     * @{
     */
    virtual bool readElement(QXmlStreamReader *xml) = 0;
    virtual void writeElement(QXmlStreamWriter *xml) = 0;
    /// @}
};

#endif // COCKATRICE_ABSTRACT_DECK_LIST_NODE_H
