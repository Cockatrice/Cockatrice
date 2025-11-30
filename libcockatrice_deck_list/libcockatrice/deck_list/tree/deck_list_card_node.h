/**
 * @file deck_list_card_node.h
 * @brief Defines the DecklistCardNode class, representing a single card entry
 *        in the deck list tree.
 *
 * DecklistCardNode is the concrete data-bearing node that corresponds to
 * an individual card entry in a deck. It stores the card’s name, quantity,
 * set information, and provider ID. These nodes live inside an
 * InnerDecklistNode (e.g., under Mainboard → Group → Card).
 */

#ifndef COCKATRICE_DECK_LIST_CARD_NODE_H
#define COCKATRICE_DECK_LIST_CARD_NODE_H

#include "abstract_deck_list_card_node.h"

#include <libcockatrice/utility/card_ref.h>

/**
 * @class DecklistCardNode
 * @ingroup DeckModels
 * @brief Concrete node type representing an actual card entry in the deck.
 *
 * This class extends AbstractDecklistCardNode to hold all information
 * needed to uniquely identify a card printing within the deck.
 *
 * ### Role in the hierarchy:
 * - Child of an InnerDecklistNode (which groups cards by zone or criteria).
 * - Leaf node in the deck tree; it does not contain further children.
 *
 * ### Data stored:
 * - @c name: Card’s display name.
 * - @c number: Quantity of this card in the deck.
 * - @c cardSetShortName: Abbreviation of the set (e.g., "NEO" for Neon Dynasty).
 * - @c cardSetNumber: Collector number within the set.
 * - @c cardProviderId: External provider identifier (e.g., UUID or MTGJSON ID).
 *
 * ### Usage:
 * - Constructed directly when building a deck list from user input or file.
 * - Used by DeckListModel to present cards in Qt views.
 * - Convertible to @c CardRef for database lookups or cross-references.
 *
 * ### Ownership:
 * - Owned by its parent InnerDecklistNode.
 * - Destroyed automatically when its parent is destroyed.
 */
class DecklistCardNode : public AbstractDecklistCardNode
{
    QString name;             ///< Display name of the card.
    int number;               ///< Quantity of this card in the deck.
    QString cardSetShortName; ///< Short set code (e.g., "NEO").
    QString cardSetNumber;    ///< Collector number within the set.
    QString cardProviderId;   ///< External provider identifier (e.g., UUID).

public:
    /**
     * @brief Construct a new DecklistCardNode.
     *
     * @param _name Display name of the card.
     * @param _number Quantity of this card (default = 1).
     * @param _parent Parent node in the tree (zone or group). May be nullptr.
     * @param position Index to insert into parent’s children. -1 = append.
     * @param _cardSetShortName Short set code (e.g., "NEO").
     * @param _cardSetNumber Collector number within the set.
     * @param _cardProviderId External provider ID (e.g., UUID).
     *
     * On construction, if a parent is provided, this node is inserted into
     * the parent’s children list automatically.
     */
    explicit DecklistCardNode(QString _name = QString(),
                              int _number = 1,
                              InnerDecklistNode *_parent = nullptr,
                              int position = -1,
                              QString _cardSetShortName = QString(),
                              QString _cardSetNumber = QString(),
                              QString _cardProviderId = QString())
        : AbstractDecklistCardNode(_parent, position), name(std::move(_name)), number(_number),
          cardSetShortName(std::move(_cardSetShortName)), cardSetNumber(std::move(_cardSetNumber)),
          cardProviderId(std::move(_cardProviderId))
    {
    }

    /**
     * @brief Copy constructor with new parent assignment.
     * @param other Existing DecklistCardNode to copy.
     * @param _parent Parent node for the copy.
     *
     * Creates a deep copy of the card node’s properties, but attaches
     * the new instance to a different parent in the tree.
     */
    explicit DecklistCardNode(DecklistCardNode *other, InnerDecklistNode *_parent);

    /// @return The quantity of this card.
    [[nodiscard]] int getNumber() const override
    {
        return number;
    }

    /// @param _number Set the quantity of this card.
    void setNumber(int _number) override
    {
        number = _number;
    }

    /// @return The display name of this card.
    [[nodiscard]] QString getName() const override
    {
        return name;
    }

    /// @param _name Set the display name of this card.
    void setName(const QString &_name) override
    {
        name = _name;
    }

    /// @return The provider identifier for this card.
    [[nodiscard]] QString getCardProviderId() const override
    {
        return cardProviderId;
    }

    /// @param _providerId Set the provider identifier for this card.
    void setCardProviderId(const QString &_providerId) override
    {
        cardProviderId = _providerId;
    }

    /// @return The short set code (e.g., "NEO").
    [[nodiscard]] QString getCardSetShortName() const override
    {
        return cardSetShortName;
    }

    /// @param _cardSetShortName Set the short set code.
    void setCardSetShortName(const QString &_cardSetShortName) override
    {
        cardSetShortName = _cardSetShortName;
    }

    /// @return The collector number of this card within its set.
    [[nodiscard]] QString getCardCollectorNumber() const override
    {
        return cardSetNumber;
    }

    /// @param _cardSetNumber Set the collector number.
    void setCardCollectorNumber(const QString &_cardSetNumber) override
    {
        cardSetNumber = _cardSetNumber;
    }

    /// @return Always false; card nodes are not deck headers.
    [[nodiscard]] bool isDeckHeader() const override
    {
        return false;
    }

    /**
     * @brief Convert this node to a CardRef.
     *
     * @return A CardRef with the card’s name and provider ID, suitable
     *         for database lookups or comparison with other card sources.
     */
    [[nodiscard]] CardRef toCardRef() const
    {
        return {name, cardProviderId};
    }
};

#endif // COCKATRICE_DECK_LIST_CARD_NODE_H
