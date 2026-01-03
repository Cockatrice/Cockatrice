#ifndef DECKLISTMODEL_H
#define DECKLISTMODEL_H

#include <../../../../libcockatrice_deck_list/libcockatrice/deck_list/tree/abstract_deck_list_card_node.h>
#include <../../../../libcockatrice_deck_list/libcockatrice/deck_list/tree/deck_list_card_node.h>
#include <QAbstractItemModel>
#include <QList>
#include <libcockatrice/card/printing/exact_card.h>
#include <libcockatrice/deck_list/deck_list.h>

class CardDatabase;
class QPrinter;
class QTextCursor;

/**
 * @namespace DeckRoles
 * @brief Custom model roles used by the DeckListModel for data retrieval.
 *
 * These roles extend Qt's item data roles starting at Qt::UserRole.
 */
namespace DeckRoles
{
/**
 * @enum DeckRoles
 * @brief Custom data roles for deck-related model items.
 *
 * These roles are used to retrieve specialized data from the DeckListModel.
 */
enum
{
    IsCardRole = Qt::UserRole + 1, /**< Indicates whether the item represents a card. */
    DepthRole,                     /**< Depth level within the deck's grouping hierarchy. */
    IsLegalRole                    /**< Whether the card is legal in the current deck format. */
};
} // namespace DeckRoles

/**
 * @namespace DeckListModelColumns
 * @brief Column indices for the DeckListModel.
 *
 * These values map to the columns in the deck list table representation.
 */
namespace DeckListModelColumns
{
/**
 * @enum DeckListModelColumns
 * @brief Column identifiers for displaying card information in the deck list.
 */
enum
{
    CARD_AMOUNT = 0,           /**< The number of copies of the card. */
    CARD_NAME = 1,             /**< The card's name. */
    CARD_SET = 2,              /**< The set or expansion the card belongs to. */
    CARD_COLLECTOR_NUMBER = 3, /**< Collector number of the card within the set. */
    CARD_PROVIDER_ID = 4       /**< ID used by the external data provider (e.g., Scryfall). */
};
} // namespace DeckListModelColumns

/**
 * @namespace DeckListModelGroupCriteria
 * @brief Specifies criteria used to group cards in the DeckListModel.
 *
 * These values determine how cards are grouped in UI views such as the deck editor.
 */
namespace DeckListModelGroupCriteria
{
/**
 * @enum DeckListModelGroupCriteria
 * @brief Available grouping strategies for deck visualization.
 */
enum Type
{
    MAIN_TYPE, /**< Group cards by their main type (e.g., creature, instant). */
    MANA_COST, /**< Group cards by their total mana cost. */
    COLOR      /**< Group cards by their color identity. */
};
static inline QString toString(Type t)
{
    switch (t) {
        case MAIN_TYPE:
            return "Main Type";
        case MANA_COST:
            return "Mana Cost";
        case COLOR:
            return "Colors";
    }
    return {};
}

static inline Type fromString(const QString &s)
{
    if (s == "Main Type")
        return MAIN_TYPE;
    if (s == "Mana Cost")
        return MANA_COST;
    if (s == "Colors")
        return COLOR;
    return MAIN_TYPE; // default
}
} // namespace DeckListModelGroupCriteria

/**
 * @class DecklistModelCardNode
 * @ingroup DeckModels
 * @brief Adapter node that wraps a DecklistCardNode for use in the DeckListModel tree.
 *
 * This class forwards all property accessors (name, number, provider ID, set info, etc.)
 * to the underlying DecklistCardNode. It exists so the model can represent cards
 * in the same hierarchy as InnerDecklistNode containers.
 */
class DecklistModelCardNode : public AbstractDecklistCardNode
{
private:
    DecklistCardNode *dataNode; /**< Pointer to the underlying data node. */

public:
    /**
     * @brief Constructs a model node wrapping a DecklistCardNode.
     * @param _dataNode The underlying DecklistCardNode to wrap.
     * @param _parent The parent InnerDecklistNode in the model tree.
     * @param position Optional position to insert in parent (-1 appends at end).
     */
    DecklistModelCardNode(DecklistCardNode *_dataNode, InnerDecklistNode *_parent, int position = -1)
        : AbstractDecklistCardNode(_parent, position), dataNode(_dataNode)
    {
    }
    [[nodiscard]] int getNumber() const override
    {
        return dataNode->getNumber();
    }
    void setNumber(int _number) override
    {
        dataNode->setNumber(_number);
    }
    [[nodiscard]] QString getName() const override
    {
        return dataNode->getName();
    }
    void setName(const QString &_name) override
    {
        dataNode->setName(_name);
    }
    [[nodiscard]] QString getCardProviderId() const override
    {
        return dataNode->getCardProviderId();
    }
    void setCardProviderId(const QString &_cardProviderId) override
    {
        dataNode->setCardProviderId(_cardProviderId);
    }
    [[nodiscard]] QString getCardSetShortName() const override
    {
        return dataNode->getCardSetShortName();
    }
    void setCardSetShortName(const QString &_cardSetShortName) override
    {
        dataNode->setCardSetShortName(_cardSetShortName);
    }
    [[nodiscard]] QString getCardCollectorNumber() const override
    {
        return dataNode->getCardCollectorNumber();
    }
    void setCardCollectorNumber(const QString &_cardSetNumber) override
    {
        dataNode->setCardCollectorNumber(_cardSetNumber);
    }
    bool getFormatLegality() const override
    {
        return dataNode->getFormatLegality();
    }
    void setFormatLegality(const bool _formatLegal) override
    {
        dataNode->setFormatLegality(_formatLegal);
    }

    /**
     * @brief Returns the underlying data node.
     * @return Pointer to the DecklistCardNode wrapped by this node.
     */
    [[nodiscard]] DecklistCardNode *getDataNode() const
    {
        return dataNode;
    }
    [[nodiscard]] bool isDeckHeader() const override
    {
        return false;
    }
};

/**
 * @class DeckListModel
 * @ingroup DeckModels
 * @brief Qt model representing a decklist for use in views (tree/table).
 *
 * DeckListModel is a QAbstractItemModel that exposes the structure of a deck
 * (zones and cards) to Qt views. It organizes cards hierarchically under
 * InnerDecklistNode containers and supports grouping, sorting, adding/removing
 * cards, and printing decklists.
 *
 * Signals:
 * - deckHashChanged(): emitted when the deck contents change in a way that
 *   affects its hash.
 *
 * Slots:
 * - rebuildTree(): rebuilds the model structure from the underlying node tree.
 */
class DeckListModel : public QAbstractItemModel
{
    Q_OBJECT

public slots:
    /**
     * @brief Rebuilds the model tree from the underlying node tree.
     *
     * This updates all indices and ensures the model reflects the current
     * state of the deck.
     */
    void rebuildTree();

    /**
     * @brief Sets the criteria used to group cards in the model.
     * @param newCriteria The new grouping criteria.
     */
    void setActiveGroupCriteria(DeckListModelGroupCriteria::Type newCriteria);

    void setActiveFormat(const QString &_format);

signals:
    /**
     * @brief Emitted whenever the deck hash changes due to modifications in the model.
     */
    void deckHashChanged();

    /**
     * @brief Emitted whenever a card is added to the deck, regardless of whether it's an entirely new card or an
     * existing card that got incremented.
     * @param index The index of the card that got added.
     */
    void cardAddedAt(const QModelIndex &index);

    /**
     * @brief Emitted whenever the deck in the model has been replaced with a new one
     */
    void deckReplaced();

public:
    explicit DeckListModel(QObject *parent = nullptr);
    explicit DeckListModel(QObject *parent, const QSharedPointer<DeckList> &deckList);
    ~DeckListModel() override;

    /**
     * @brief Returns the root index of the model.
     * @return QModelIndex representing the root node.
     */
    [[nodiscard]] QModelIndex getRoot() const
    {
        return nodeToIndex(root);
    }

    /// @name Qt model overrides
    ///@{
    [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
    [[nodiscard]] int columnCount(const QModelIndex & /*parent*/ = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    [[nodiscard]] QModelIndex parent(const QModelIndex &index) const override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;
    void sort(int column, Qt::SortOrder order) override;
    ///@}

    /**
     * @brief Finds a card by name, zone, and optional identifiers.
     * @param cardName The card's name.
     * @param zoneName The zone to search in (main/side/etc.).
     * @param providerId Optional provider-specific ID.
     * @param cardNumber Optional collector number.
     * @return QModelIndex of the card, or invalid index if not found.
     */
    [[nodiscard]] QModelIndex findCard(const QString &cardName,
                                       const QString &zoneName,
                                       const QString &providerId = "",
                                       const QString &cardNumber = "") const;

    /**
     * @brief Adds a card using the preferred printing if available.
     *
     * @param cardName Name of the card to add.
     * @param zoneName Zone to insert the card into.
     * @param abAddAnyway Whether to add the card even if resolution fails.
     * @return QModelIndex pointing to the newly inserted card node.
     */
    QModelIndex addPreferredPrintingCard(const QString &cardName, const QString &zoneName, bool abAddAnyway);

    /**
     * @brief Adds an ExactCard to the specified zone.
     * @param card The card to add.
     * @param zoneName The zone to insert the card into.
     * @return QModelIndex pointing to the newly inserted card node.
     */
    QModelIndex addCard(const ExactCard &card, const QString &zoneName);

    /**
     * @brief Changes the `amount` field in the card node at the index by the amount.
     * Removes the node if it causes the amount to fall to 0 or below.
     * @param idx The index of a card node. No-ops if the index is invalid or not a card node.
     * @param offset The amount to change the amount field by.
     * @return Whether the operation was successful
     */
    bool offsetCountAtIndex(const QModelIndex &idx, int offset);

    /**
     * @brief Removes all cards and resets the model.
     */
    void cleanList();

    [[nodiscard]] QSharedPointer<DeckList> getDeckList() const
    {
        return deckList;
    }
    void setDeckList(const QSharedPointer<DeckList> &_deck);

    /**
     * @brief Apply a function to every card in the deck tree.
     *
     * @param func Function taking (zone node, card node).
     */
    void forEachCard(const std::function<void(InnerDecklistNode *, DecklistCardNode *)> &func);

    /**
     * @brief Gets a list of all card nodes in the deck.
     */
    [[nodiscard]] QList<const DecklistCardNode *> getCardNodes() const;
    [[nodiscard]] QList<const DecklistCardNode *> getCardNodesForZone(const QString &zoneName) const;

    /**
     * @brief Gets a deduplicated list of all card names that appear in the model
     */
    [[nodiscard]] QList<QString> getCardNames() const;
    /**
     * @brief Gets a deduplicated list of all CardRefs that appear in the model
     */
    [[nodiscard]] QList<CardRef> getCardRefs() const;
    /**
     * @brief Gets a list of all zone names that appear in the model
     */
    [[nodiscard]] QList<QString> getZones() const;

private:
    QSharedPointer<DeckList> deckList; /**< Pointer to the decklist providing the underlying data. */
    InnerDecklistNode *root;           /**< Root node of the model tree. */
    DeckListModelGroupCriteria::Type activeGroupCriteria = DeckListModelGroupCriteria::MAIN_TYPE;
    int lastKnownColumn;          /**< Last column used for sorting. */
    Qt::SortOrder lastKnownOrder; /**< Last known sort order. */

    InnerDecklistNode *createNodeIfNeeded(const QString &name, InnerDecklistNode *parent);
    QModelIndex nodeToIndex(AbstractDecklistNode *node) const;
    [[nodiscard]] DecklistModelCardNode *findCardNode(const QString &cardName,
                                                      const QString &zoneName,
                                                      const QString &providerId = "",
                                                      const QString &cardNumber = "") const;

    /**
     * @brief Determines the sorted insertion row for a card.
     * @param parent The parent node where the card will be inserted.
     * @param cardInfo The card info to insert.
     * @return Row index where the card should be inserted to maintain sort order.
     */
    int findSortedInsertRow(const InnerDecklistNode *parent, const CardInfoPtr &cardInfo) const;

    /**
     * @brief Recursively emits the dataChanged signal with role as Qt::BackgroundRole for all indices that are children
     * of the given node. This is used to update the background color when changing formats.
     * @param parent The parent node
     */
    void emitBackgroundUpdates(const QModelIndex &parent);

    /**
     * @brief Recursively emits the dataChanged signal for the given node and all parent nodes.
     * @param index The parent node
     */
    void emitRecursiveUpdates(const QModelIndex &index);

    void sortHelper(InnerDecklistNode *node, Qt::SortOrder order);

    template <typename T> T getNode(const QModelIndex &index) const
    {
        if (!index.isValid())
            return dynamic_cast<T>(root);
        return dynamic_cast<T>(static_cast<AbstractDecklistNode *>(index.internalPointer()));
    }

    void refreshCardFormatLegalities();
};

#endif
