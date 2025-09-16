#ifndef DECKLISTMODEL_H
#define DECKLISTMODEL_H

#include "../card/exact_card.h"
#include "abstract_deck_list_card_node.h"
#include "deck_list.h"
#include "deck_list_card_node.h"

#include <QAbstractItemModel>
#include <QList>

class DeckLoader;
class CardDatabase;
class QPrinter;
class QTextCursor;

/**
 * @brief Specifies the criteria used to group cards in the DeckListModel.
 */
enum DeckListModelGroupCriteria
{
    MAIN_TYPE, /**< Group cards by their main type (e.g., creature, instant). */
    MANA_COST, /**< Group cards by their mana cost. */
    COLOR      /**< Group cards by their color identity. */
};

/**
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
    int getNumber() const override
    {
        return dataNode->getNumber();
    }
    void setNumber(int _number) override
    {
        dataNode->setNumber(_number);
    }
    QString getName() const override
    {
        return dataNode->getName();
    }
    void setName(const QString &_name) override
    {
        dataNode->setName(_name);
    }
    QString getCardProviderId() const override
    {
        return dataNode->getCardProviderId();
    }
    void setCardProviderId(const QString &_cardProviderId) override
    {
        dataNode->setCardProviderId(_cardProviderId);
    }
    QString getCardSetShortName() const override
    {
        return dataNode->getCardSetShortName();
    }
    void setCardSetShortName(const QString &_cardSetShortName) override
    {
        dataNode->setCardSetShortName(_cardSetShortName);
    }
    QString getCardCollectorNumber() const override
    {
        return dataNode->getCardCollectorNumber();
    }
    void setCardCollectorNumber(const QString &_cardSetNumber) override
    {
        dataNode->setCardCollectorNumber(_cardSetNumber);
    }

    /**
     * @brief Returns the underlying data node.
     * @return Pointer to the DecklistCardNode wrapped by this node.
     */
    DecklistCardNode *getDataNode() const
    {
        return dataNode;
    }
    [[nodiscard]] bool isDeckHeader() const override
    {
        return false;
    }
};

/**
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
 * - rebuildTree(): rebuilds the model structure from the underlying DeckLoader.
 * - printDeckList(): renders the decklist to a QPrinter.
 */
class DeckListModel : public QAbstractItemModel
{
    Q_OBJECT
private slots:
    /**
     * @brief Rebuilds the model tree from the underlying DeckLoader.
     *
     * This updates all indices and ensures the model reflects the current
     * state of the deck.
     */
    void rebuildTree();

public slots:
    /**
     * @brief Prints the decklist to the provided QPrinter.
     * @param printer The printer to render the decklist to.
     */
    void printDeckList(QPrinter *printer);

signals:
    /**
     * @brief Emitted whenever the deck hash changes due to modifications in the model.
     */
    void deckHashChanged();

public:
    explicit DeckListModel(QObject *parent = nullptr);
    ~DeckListModel() override;

    /**
     * @brief Returns the root index of the model.
     * @return QModelIndex representing the root node.
     */
    QModelIndex getRoot() const
    {
        return nodeToIndex(root);
    };

    /**
     * @brief Returns the value of the grouping category for a card based on the current criteria.
     * @param info Pointer to card information.
     * @return String representing the value of the current grouping criteria for the card.
     */
    QString getGroupCriteriaForCard(CardInfoPtr info) const;

    // Qt model overrides
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex & /*parent*/ = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;

    /**
     * @brief Finds a card by name, zone, and optional identifiers.
     * @param cardName The card's name.
     * @param zoneName The zone to search in (main/side/etc.).
     * @param providerId Optional provider-specific ID.
     * @param cardNumber Optional collector number.
     * @return QModelIndex of the card, or invalid index if not found.
     */
    QModelIndex findCard(const QString &cardName,
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
     * @brief Determines the sorted insertion row for a card.
     * @param parent The parent node where the card will be inserted.
     * @param cardInfo The card info to insert.
     * @return Row index where the card should be inserted to maintain sort order.
     */
    int findSortedInsertRow(InnerDecklistNode *parent, CardInfoPtr cardInfo) const;

    void sort(int column, Qt::SortOrder order) override;

    /**
     * @brief Removes all cards and resets the model.
     */
    void cleanList();
    DeckLoader *getDeckList() const
    {
        return deckList;
    }
    void setDeckList(DeckLoader *_deck);

    QList<ExactCard> getCards() const;
    QList<ExactCard> getCardsForZone(const QString &zoneName) const;
    QList<QString> *getZones() const;

    /**
     * @brief Sets the criteria used to group cards in the model.
     * @param newCriteria The new grouping criteria.
     */
    void setActiveGroupCriteria(DeckListModelGroupCriteria newCriteria);

private:
    DeckLoader *deckList;    /**< Pointer to the deck loader providing the underlying data. */
    InnerDecklistNode *root; /**< Root node of the model tree. */
    DeckListModelGroupCriteria activeGroupCriteria = DeckListModelGroupCriteria::MAIN_TYPE;
    int lastKnownColumn;          /**< Last column used for sorting. */
    Qt::SortOrder lastKnownOrder; /**< Last known sort order. */

    InnerDecklistNode *createNodeIfNeeded(const QString &name, InnerDecklistNode *parent);
    QModelIndex nodeToIndex(AbstractDecklistNode *node) const;
    DecklistModelCardNode *findCardNode(const QString &cardName,
                                        const QString &zoneName,
                                        const QString &providerId = "",
                                        const QString &cardNumber = "") const;
    void emitRecursiveUpdates(const QModelIndex &index);
    void sortHelper(InnerDecklistNode *node, Qt::SortOrder order);

    void printDeckListNode(QTextCursor *cursor, InnerDecklistNode *node);

    template <typename T> T getNode(const QModelIndex &index) const
    {
        if (!index.isValid())
            return dynamic_cast<T>(root);
        return dynamic_cast<T>(static_cast<AbstractDecklistNode *>(index.internalPointer()));
    }
};

#endif
