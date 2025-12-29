/**
 * @file inner_deck_list_node.h
 * @brief Defines the InnerDecklistNode class, which represents
 *        structural nodes (zones and groups) in the deck tree.
 *
 * The deck tree consists of:
 * - A root node (invisible).
 * - Zones (Main, Sideboard, Tokens).
 * - Optional grouping nodes (e.g., by type, color, or mana cost).
 * - Card nodes as leaves.
 *
 * InnerDecklistNode implements the zone/group nodes and provides
 * storage and management of child nodes.
 */

#ifndef COCKATRICE_INNER_DECK_LIST_NODE_H
#define COCKATRICE_INNER_DECK_LIST_NODE_H

#include "abstract_deck_list_node.h"

/// Constant for the "main" deck zone name.
#define DECK_ZONE_MAIN "main"
/// Constant for the "sideboard" zone name.
#define DECK_ZONE_SIDE "side"
/// Constant for the "tokens" zone name.
#define DECK_ZONE_TOKENS "tokens"

/**
 * @class InnerDecklistNode
 * @brief Represents a container node in the deck list hierarchy
 *        (zones and groupings).
 *
 * Unlike DecklistCardNode, which holds leaf card data, this class
 * manages collections of child nodes, which may themselves be
 * InnerDecklistNode or DecklistCardNode objects.
 *
 * ### Role in the hierarchy:
 * - Root node (invisible): Holds zones.
 * - Zone nodes: "main", "side", "tokens".
 * - Grouping nodes: Created dynamically when grouping by type,
 *   color, or mana cost.
 * - Card nodes: Always children of an InnerDecklistNode.
 *
 * ### Design notes:
 * - Inherits from AbstractDecklistNode (tree interface) and
 *   QList<AbstractDecklistNode*> (storage of children).
 *   This allows direct QList-style manipulation of children while
 *   still presenting a polymorphic node interface.
 *
 * ### Responsibilities:
 * - Store a display name.
 * - Own and manage child nodes (insert, clear, find).
 * - Provide recursive operations such as counting cards or computing height.
 * - Implement sorting logic for reordering children.
 * - Implement XML serialization for persistence.
 *
 * ### Ownership:
 * - Owns all child nodes stored in the QList. The destructor
 *   recursively deletes children.
 */
class InnerDecklistNode : public AbstractDecklistNode, public QList<AbstractDecklistNode *>
{
    QString name; ///< Internal identifier for this node (zone or group name).

public:
    /**
     * @brief Construct a new InnerDecklistNode.
     *
     * @param _name Internal name (e.g., "main", "side", "tokens", or group label).
     * @param _parent Parent node (may be nullptr for the root).
     * @param position Optional index for insertion into parent. -1 = append.
     */
    explicit InnerDecklistNode(QString _name = QString(), InnerDecklistNode *_parent = nullptr, int position = -1)
        : AbstractDecklistNode(_parent, position), name(std::move(_name))
    {
    }

    /**
     * @brief Copy constructor with parent reassignment.
     * @param other Node to copy from (deep copy of children).
     * @param _parent Parent node for the copy.
     */
    explicit InnerDecklistNode(InnerDecklistNode *other, InnerDecklistNode *_parent = nullptr);

    /**
     * @brief Destructor. Recursively deletes all child nodes.
     */
    ~InnerDecklistNode() override;

    /**
     * @brief Set the sorting method for this node and all children.
     * @param method Sort method to apply recursively.
     */
    void setSortMethod(DeckSortMethod method) override;

    /// @return The internal name of this node.
    [[nodiscard]] QString getName() const override
    {
        return name;
    }

    /// @param _name Set the internal name of this node.
    void setName(const QString &_name)
    {
        name = _name;
    }

    /**
     * @brief Translate an internal name into a user-visible name.
     *
     * For example, the internal string "main" is presented as
     * "Mainboard" in the UI.
     *
     * @param _name Internal identifier.
     * @return Display-friendly string.
     */
    static QString visibleNameFromName(const QString &_name);

    /**
     * @brief Get this node’s display-friendly name.
     * @return Human-readable name (zone/group name).
     */
    [[nodiscard]] virtual QString getVisibleName() const;

    /// @return Always empty for container nodes.
    [[nodiscard]] QString getCardProviderId() const override
    {
        return "";
    }

    /// @return Always empty for container nodes.
    [[nodiscard]] QString getCardSetShortName() const override
    {
        return "";
    }

    /// @return Always empty for container nodes.
    [[nodiscard]] QString getCardCollectorNumber() const override
    {
        return "";
    }

    /// @return Always true; InnerDecklistNode represents deck structure.
    [[nodiscard]] bool isDeckHeader() const override
    {
        return true;
    }

    /**
     * @brief Delete all children of this node, recursively.
     */
    void clearTree();

    /**
     * @brief Find a direct child node by name.
     * @param _name Name to match.
     * @return Pointer to child node, or nullptr if not found.
     */
    AbstractDecklistNode *findChild(const QString &_name);

    /**
     * @brief Find a child card node by name, provider ID, and collector number.
     *
     * Searches immediate children only.
     *
     * @param _name Card name to match.
     * @param _providerId Optional provider ID to match.
     * @param _cardNumber Optional collector number to match.
     * @return Pointer to child node if found, nullptr otherwise.
     */
    AbstractDecklistNode *findCardChildByNameProviderIdAndNumber(const QString &_name,
                                                                 const QString &_providerId = "",
                                                                 const QString &_cardNumber = "");

    /**
     * @brief Compute the height of this node.
     * @return Maximum depth of descendants + 1.
     */
    [[nodiscard]] int height() const override;

    /**
     * @brief Count cards recursively under this node.
     * @param countTotalCards If true, sums up quantities of cards.
     *        If false, counts unique card nodes.
     * @return Total count.
     */
    [[nodiscard]] int recursiveCount(bool countTotalCards = false) const;

    /**
     * @brief Compare this node against another for sorting.
     *
     * Uses current @c sortMethod to determine the comparison.
     *
     * @param other Node to compare.
     * @return true if this node should sort before @p other.
     */
    bool compare(AbstractDecklistNode *other) const override;

    /// @copydoc compare(AbstractDecklistNode*) const
    bool compareNumber(AbstractDecklistNode *other) const;

    /// @copydoc compare(AbstractDecklistNode*) const
    bool compareName(AbstractDecklistNode *other) const;

    /**
     * @brief Sort this node’s children recursively.
     *
     * @param order Ascending or descending.
     * @return A QVector of (oldIndex, newIndex) pairs indicating
     *         how children were reordered.
     */
    QVector<QPair<int, int>> sort(Qt::SortOrder order = Qt::AscendingOrder);

    /**
     * @brief Deserialize this node and its children from XML.
     * @param xml Reader positioned at this element.
     * @return true if parsing succeeded.
     */
    bool readElement(QXmlStreamReader *xml) override;

    /**
     * @brief Serialize this node and its children to XML.
     * @param xml Writer to append elements to.
     */
    void writeElement(QXmlStreamWriter *xml) override;
};

#endif // COCKATRICE_INNER_DECK_LIST_NODE_H
