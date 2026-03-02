#ifndef COCKATRICE_DECKLIST_NODE_TREE_H
#define COCKATRICE_DECKLIST_NODE_TREE_H

#include "libcockatrice/utility/card_ref.h"
#include "tree/deck_list_card_node.h"
#include "tree/inner_deck_list_node.h"

#include <QSet>

class DecklistNodeTree
{
    InnerDecklistNode *root; ///< Root of the deck tree (zones + cards).

public:
    /// @brief Constructs an empty DecklistNodeTree
    explicit DecklistNodeTree();
    /// @brief Copy constructor. Deep copies the tree
    explicit DecklistNodeTree(const DecklistNodeTree &other);
    /// @brief Copy-assignment operator. Deep copies the tree
    DecklistNodeTree &operator=(const DecklistNodeTree &other);

    virtual ~DecklistNodeTree();

    /**
     * @brief Gets a pointer to the underlying root node.
     * Note: DO NOT call this method unless the object needs to have access to the underlying model.
     * For now, only the DeckListModel should be calling this.
     */
    InnerDecklistNode *getRoot() const
    {
        return root;
    }

    bool isEmpty() const;

    /**
     * @brief Deletes all nodes except the root.
     */
    void clear();

    /**
     * Gets all card nodes in the tree
     * @param restrictToZones Only get the nodes in these zones
     * @return A QList containing all the card nodes in the zone.
     */
    QList<const DecklistCardNode *> getCardNodes(const QSet<QString> &restrictToZones = {}) const;

    /**
     * Gets all zone nodes in the tree
     * @param restrictToZones If not empty, only get the zone nodes with these names.
     * @return A QList containing all the zone nodes in the tree.
     */
    QList<const InnerDecklistNode *> getZoneNodes(const QSet<QString> &restrictToZones = {}) const;

    /**
     * @brief Computes the deck hash
     */
    QString computeDeckHash() const;

    /**
     *@brief Writes the contents of the deck to xml
     */
    void write(QXmlStreamWriter *xml) const;

    /**
     * @brief Reads a "zone" section of the xml to this tree
     */
    void readZoneElement(QXmlStreamReader *xml);

    DecklistCardNode *addCard(const QString &cardName,
                              int amount,
                              const QString &zoneName,
                              int position,
                              const QString &cardSetName = QString(),
                              const QString &cardSetCollectorNumber = QString(),
                              const QString &cardProviderId = QString(),
                              const bool formatLegal = true);
    bool deleteNode(AbstractDecklistNode *node, InnerDecklistNode *rootNode = nullptr);

    /**
     * @brief Apply a function to every card in the deck tree. This can modify the cards.
     *
     * @param func Function taking (zone node, card node).
     */
    void forEachCard(const std::function<void(InnerDecklistNode *, DecklistCardNode *)> &func) const;

private:
    // Helpers for traversing the tree
    InnerDecklistNode *getZoneObjFromName(const QString &zoneName) const;
};

#endif // COCKATRICE_DECKLIST_NODE_TREE_H
