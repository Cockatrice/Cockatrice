/**
 * @file abstract_deck_list_card_node.h
 * @brief Defines the AbstractDecklistCardNode base class, which adds
 *        card-specific behavior on top of AbstractDecklistNode.
 *
 * This class is the intermediate abstract base between the generic
 * AbstractDecklistNode and concrete card entries such as DecklistCardNode
 * or DecklistModelCardNode.
 */

#ifndef COCKATRICE_ABSTRACT_DECK_LIST_CARD_NODE_H
#define COCKATRICE_ABSTRACT_DECK_LIST_CARD_NODE_H

#include "abstract_deck_list_node.h"

/**
 * @class AbstractDecklistCardNode
 * @ingroup DeckModels
 * @brief Abstract base class for all deck list nodes that represent
 *        actual card entries.
 *
 * While AbstractDecklistNode provides the general interface for all
 * nodes in the deck tree (zones, groups, cards), this subclass refines
 * the interface to cover properties specific to *cards*:
 * - Quantity (number of copies).
 * - Name.
 * - Set code and collector number.
 * - Provider ID.
 *
 * ### Role in the hierarchy:
 * - Leaf-oriented abstract class; no children of its own.
 * - Serves as the base for concrete implementations:
 *   - @c DecklistCardNode: Stores real card data in the deck tree.
 *   - @c DecklistModelCardNode: Wraps a DecklistCardNode for use
 *     in the Qt model layer.
 *
 * ### Responsibilities:
 * - Defines getters/setters for all card-identifying attributes.
 * - Provides comparison logic for sorting by name or number.
 * - Implements XML serialization for saving/loading deck files.
 *
 * ### Ownership:
 * - As with all nodes, owned by its parent InnerDecklistNode.
 */
class AbstractDecklistCardNode : public AbstractDecklistNode
{
public:
    /**
     * @brief Construct a new AbstractDecklistCardNode.
     *
     * @param _parent Optional parent node. If provided, this node
     *        will be inserted into the parent’s children list.
     * @param position Index at which to insert into parent’s children.
     *        If -1, the node is appended to the end.
     */
    explicit AbstractDecklistCardNode(InnerDecklistNode *_parent = nullptr, int position = -1)
        : AbstractDecklistNode(_parent, position)
    {
    }

    /// @return The number of copies of this card in the deck.
    virtual int getNumber() const = 0;

    /// @param _number Set the number of copies of this card.
    virtual void setNumber(int _number) = 0;

    /// @return The display name of this card.
    QString getName() const override = 0;

    /// @param _name Set the display name of this card.
    virtual void setName(const QString &_name) = 0;

    /// @return The provider identifier for this card (e.g., UUID).
    virtual QString getCardProviderId() const override = 0;

    /// @param _cardProviderId Set the provider identifier for this card.
    virtual void setCardProviderId(const QString &_cardProviderId) = 0;

    /// @return The abbreviated set code (e.g., "NEO").
    virtual QString getCardSetShortName() const override = 0;

    /// @param _cardSetShortName Set the abbreviated set code.
    virtual void setCardSetShortName(const QString &_cardSetShortName) = 0;

    /// @return The collector number of the card within its set.
    virtual QString getCardCollectorNumber() const override = 0;

    /// @param _cardSetNumber Set the collector number.
    virtual void setCardCollectorNumber(const QString &_cardSetNumber) = 0;

    /**
     * @brief Get the height of this node in the tree.
     *
     * For card nodes, height is always 0 because they are leaf nodes
     * and do not contain children.
     *
     * @return 0
     */
    int height() const override
    {
        return 0;
    }

    /**
     * @brief Compare this card node against another for sorting.
     *
     * Uses the node’s current @c sortMethod to determine how to compare:
     * - ByName: Alphabetical comparison.
     * - ByNumber: Numerical comparison.
     * - Default: Falls back to implementation-defined behavior.
     *
     * @param other Another node to compare against.
     * @return true if this node should sort before @p other.
     */
    bool compare(AbstractDecklistNode *other) const override;

    /**
     * @brief Compare this card node to another by quantity.
     * @param other Node to compare against.
     * @return true if this node’s number < other’s number.
     */
    bool compareNumber(AbstractDecklistNode *other) const;

    /**
     * @brief Compare this card node to another by name.
     * @param other Node to compare against.
     * @return true if this node’s name comes before other’s name.
     */
    bool compareName(AbstractDecklistNode *other) const;

    /**
     * @brief Deserialize this node’s properties from XML.
     * @param xml QXmlStreamReader positioned at the element.
     * @return true if parsing succeeded.
     *
     * This supports loading deck files from Cockatrice’s XML format.
     */
    bool readElement(QXmlStreamReader *xml) override;

    /**
     * @brief Serialize this node’s properties to XML.
     * @param xml Writer to append this node’s XML element.
     *
     * This supports saving deck files to Cockatrice’s XML format.
     */
    void writeElement(QXmlStreamWriter *xml) override;
};

#endif // COCKATRICE_ABSTRACT_DECK_LIST_CARD_NODE_H
