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
    /** @brief Constructs an empty DecklistNodeTree. */
    explicit DecklistNodeTree();
    /** @brief Copy constructor. Deep copies the tree. */
    explicit DecklistNodeTree(const DecklistNodeTree &other);
    /** @brief Copy-assignment operator. Deep copies the tree. */
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
     * @brief Creates a new custom zone nested under a board zone.
     *
     * Custom zone names must be unique across the whole deck so that cards can be
     * added to a custom zone without specifying its board zone.
     *
     * @param boardZoneName Name of the board zone (e.g. DECK_ZONE_MAIN).
     * @param zoneName Name of the custom zone.
     * @return The created zone node, or nullptr if the name is already in use.
     */
    InnerDecklistNode *addCustomZone(const QString &boardZoneName, const QString &zoneName);

    /**
     * @brief Renames a custom zone.
     * @return true on success, false if the zone was not found or the new name is taken.
     */
    bool renameCustomZone(const QString &oldZoneName, const QString &newZoneName);

    /**
     * @brief Moves a custom zone (and all its cards) to another board zone.
     * @return true on success, false if the zone or the new board zone was not found.
     */
    bool moveCustomZone(const QString &zoneName, const QString &newBoardZoneName);

    /**
     * @brief Removes a custom zone and all its cards.
     * @return true if the zone was found and removed.
     */
    bool removeCustomZone(const QString &zoneName);

    /**
     * @brief Gets all custom zones nested under a board zone.
     * @param boardZoneName Name of the board zone.
     * @return The custom zones, in insertion order.
     */
    QList<const InnerDecklistNode *> getCustomZones(const QString &boardZoneName) const;

    /**
     * @brief Applies a function to every card in the deck tree. This can modify the cards.
     *
     * @param func Function taking (top-level board zone node, card node). Cards nested
     *        in custom zones are reported with their board zone.
     */
    void forEachCard(const std::function<void(InnerDecklistNode *, DecklistCardNode *)> &func) const;

private:
    // Helpers for traversing the tree
    InnerDecklistNode *getZoneObjFromName(const QString &zoneName) const;
    InnerDecklistNode *findCustomZoneByName(const QString &zoneName) const;
    bool hasZoneName(const QString &zoneName) const;
};

#endif // COCKATRICE_DECKLIST_NODE_TREE_H
