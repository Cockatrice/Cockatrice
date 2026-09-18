#include "deck_list_node_tree.h"

#include "tree/deck_list_card_node.h"

#include <QCryptographicHash>
#include <QSet>

static constexpr int MAX_DECK_SIZE = 1e5;

namespace
{

/**
 * @brief Expands card nodes into one lowercase name entry per copy.
 *
 * @param nodes The card nodes to expand.
 * @param prefix Optional prefix prepended to every entry (e.g. "SB:" for the sideboard).
 * @return One entry per copy, in node order.
 */
QStringList cardNodesToCopies(const QList<const DecklistCardNode *> &nodes, const QString &prefix = {})
{
    QStringList result;
    for (auto node : nodes) {
        for (int i = 0; i < node->getNumber(); ++i) {
            result.append(prefix + node->getName().toLower());
        }
    }
    return result;
}

/**
 * @brief Packs the first five bytes of a SHA-1 digest into a compact base-32 number.
 *
 * The bytes are placed in most-significant-byte-first order (byte 0 shifted by 32,
 * byte 4 unshifted) so decks that only differ in their low-order hash bytes still
 * produce distinct identifiers.
 *
 * @return The 8-character base-32 representation of the packed number.
 */
QString encodeDeckHash(const QByteArray &digest)
{
    quint64 number = 0;
    for (int i = 0; i < 5; ++i) {
        number |= static_cast<quint64>(static_cast<unsigned char>(digest[i])) << (32 - 8 * i);
    }
    return QString::number(number, 32).rightJustified(8, '0');
}

/**
 * @brief Collects every card node in @p node's subtree, in tree order.
 *
 * @return The collected card nodes.
 */
QList<const DecklistCardNode *> collectCardsRecursive(const InnerDecklistNode *node)
{
    QList<const DecklistCardNode *> result;
    for (int i = 0; i < node->size(); i++) {
        if (auto *card = dynamic_cast<const DecklistCardNode *>(node->at(i))) {
            result.append(card);
        } else if (auto *inner = dynamic_cast<const InnerDecklistNode *>(node->at(i))) {
            result.append(collectCardsRecursive(inner));
        }
    }
    return result;
}

/**
 * @brief Invokes @p func on every card in @p node's subtree.
 *
 * Cards nested in custom zones are reported with their top-level @p boardZone
 * so that callers can classify cards by board (main/side/maybeboard/tokens).
 */
void forEachCardInNode(InnerDecklistNode *boardZone,
                       InnerDecklistNode *node,
                       const std::function<void(InnerDecklistNode *, DecklistCardNode *)> &func)
{
    for (int i = 0; i < node->size(); i++) {
        if (auto *card = dynamic_cast<DecklistCardNode *>(node->at(i))) {
            func(boardZone, card);
        } else if (auto *inner = dynamic_cast<InnerDecklistNode *>(node->at(i))) {
            forEachCardInNode(boardZone, inner, func);
        }
    }
}

} // namespace

DecklistNodeTree::DecklistNodeTree() : root(new InnerDecklistNode())
{
}

DecklistNodeTree::DecklistNodeTree(const DecklistNodeTree &other) : root(new InnerDecklistNode(other.root))
{
}

DecklistNodeTree &DecklistNodeTree::operator=(const DecklistNodeTree &other)
{
    if (this != &other) {
        delete root;
        root = new InnerDecklistNode(other.root);
    }
    return *this;
}

DecklistNodeTree::~DecklistNodeTree()
{
    delete root;
}

bool DecklistNodeTree::isEmpty() const
{
    return root->isEmpty();
}

void DecklistNodeTree::clear()
{
    root->clearTree();
}

QList<const DecklistCardNode *> DecklistNodeTree::getCardNodes(const QSet<QString> &restrictToZones) const
{
    QList<const DecklistCardNode *> result;

    for (auto *zoneNode : getZoneNodes(restrictToZones)) {
        result.append(collectCardsRecursive(zoneNode));
    }

    return result;
}

QList<const InnerDecklistNode *> DecklistNodeTree::getZoneNodes(const QSet<QString> &restrictToZones) const
{
    QList<const InnerDecklistNode *> zones;
    for (auto *node : *root) {
        InnerDecklistNode *currentZone = dynamic_cast<InnerDecklistNode *>(node);
        if (!currentZone) {
            continue;
        }
        if (!restrictToZones.isEmpty() && !restrictToZones.contains(currentZone->getName())) {
            continue;
        }
        zones.append(currentZone);
    }

    return zones;
}

QString DecklistNodeTree::computeDeckHash() const
{
    auto mainDeckNodes = getCardNodes({DECK_ZONE_MAIN});
    auto sideDeckNodes = getCardNodes({DECK_ZONE_SIDE});

    QStringList cardList = cardNodesToCopies(mainDeckNodes) + cardNodesToCopies(sideDeckNodes, "SB:");

    cardList.sort();
    QByteArray deckHashArray = QCryptographicHash::hash(cardList.join(";").toUtf8(), QCryptographicHash::Sha1);
    return encodeDeckHash(deckHashArray);
}

void DecklistNodeTree::write(QXmlStreamWriter *xml) const
{
    for (int i = 0; i < root->size(); i++) {
        root->at(i)->writeElement(xml);
    }
}

void DecklistNodeTree::readZoneElement(QXmlStreamReader *xml)
{
    QString zoneName = xml->attributes().value("name").toString();
    InnerDecklistNode *newZone = getZoneObjFromName(zoneName);
    totalCards += newZone->readElement(xml, MAX_DECK_SIZE - totalCards);
}

DecklistCardNode *DecklistNodeTree::addCard(const QString &cardName,
                                            int amount,
                                            const QString &zoneName,
                                            int position,
                                            const QString &cardSetName,
                                            const QString &cardSetCollectorNumber,
                                            const QString &cardProviderId,
                                            const bool formatLegal)
{
    amount = qMin(amount, MAX_DECK_SIZE - totalCards);
    totalCards += amount;
    auto *zoneNode = getZoneObjFromName(zoneName);
    auto *node = new DecklistCardNode(cardName, amount, zoneNode, position, cardSetName, cardSetCollectorNumber,
                                      cardProviderId, formatLegal);
    return node;
}

bool DecklistNodeTree::deleteNode(AbstractDecklistNode *node, InnerDecklistNode *rootNode)
{
    if (node == root) {
        return true;
    }

    if (rootNode == nullptr) {
        rootNode = root;
    }

    int index = rootNode->indexOf(node);
    if (index != -1) {
        delete rootNode->takeAt(index);

        // Empty custom zones are kept while empty board zones get pruned.
        if (rootNode->empty() && rootNode->getParent() == root) {
            deleteNode(rootNode, rootNode->getParent());
        }

        return true;
    }

    for (int i = 0; i < rootNode->size(); i++) {
        auto *inner = dynamic_cast<InnerDecklistNode *>(rootNode->at(i));
        if (inner) {
            if (deleteNode(node, inner)) {
                return true;
            }
        }
    }

    return false;
}

void DecklistNodeTree::forEachCard(const std::function<void(InnerDecklistNode *, DecklistCardNode *)> &func) const
{
    for (int i = 0; i < root->size(); i++) {
        if (auto *zone = dynamic_cast<InnerDecklistNode *>(root->at(i))) {
            forEachCardInNode(zone, zone, func);
        }
    }
}

/**
 * Gets the InnerDecklistNode that is the root node for the given zone, creating a new node if it doesn't exist.
 *
 * Top-level zones take precedence, then deck-unique custom zones nested under boards
 * are resolved. Unknown names create a new top-level zone (legacy behavior).
 */
InnerDecklistNode *DecklistNodeTree::getZoneObjFromName(const QString &zoneName)
{
    for (int i = 0; i < root->size(); i++) {
        auto *node = dynamic_cast<InnerDecklistNode *>(root->at(i));
        if (node && node->getName() == zoneName) {
            return node;
        }
    }

    if (auto *customZone = findCustomZoneByName(zoneName)) {
        return customZone;
    }

    return new InnerDecklistNode(zoneName, root);
}

InnerDecklistNode *DecklistNodeTree::findBoardZone(const QString &boardZoneName) const
{
    return dynamic_cast<InnerDecklistNode *>(root->findChild(boardZoneName));
}

InnerDecklistNode *DecklistNodeTree::findOrCreateBoardZone(const QString &boardZoneName)
{
    auto *boardZone = findBoardZone(boardZoneName);
    if (!boardZone &&
        (boardZoneName == DECK_ZONE_MAYBEBOARD || boardZoneName == DECK_ZONE_MAIN || boardZoneName == DECK_ZONE_SIDE)) {
        // The boards are lazy zones: they only exist once cards or custom zones need them.
        boardZone = new InnerDecklistNode(boardZoneName, root);
    }
    return boardZone;
}

InnerDecklistNode *DecklistNodeTree::addCustomZone(const QString &boardZoneName, const QString &zoneName)
{
    if (hasZoneName(zoneName)) {
        return nullptr;
    }

    auto *boardZone = findOrCreateBoardZone(boardZoneName);

    if (!boardZone) {
        return nullptr;
    }

    return new InnerDecklistNode(zoneName, boardZone);
}

bool DecklistNodeTree::renameCustomZone(const QString &oldZoneName, const QString &newZoneName)
{
    if (hasZoneName(newZoneName)) {
        return false;
    }

    auto *zone = findCustomZoneByName(oldZoneName);
    if (!zone) {
        return false;
    }

    zone->setName(newZoneName);
    return true;
}

bool DecklistNodeTree::moveCustomZone(const QString &zoneName, const QString &newBoardZoneName)
{
    auto *zone = findCustomZoneByName(zoneName);
    if (!zone) {
        return false;
    }

    auto *currentBoardZone = zone->getParent();
    if (currentBoardZone && currentBoardZone->getName() == newBoardZoneName) {
        return true;
    }

    auto *newBoardZone = findOrCreateBoardZone(newBoardZoneName);
    if (!newBoardZone) {
        return false;
    }

    currentBoardZone->removeOne(zone);
    newBoardZone->append(zone);
    zone->setParent(newBoardZone);
    return true;
}

bool DecklistNodeTree::removeCustomZone(const QString &zoneName)
{
    auto *zone = findCustomZoneByName(zoneName);
    if (!zone) {
        return false;
    }

    // Detach and delete without pruning the board zone.
    auto *boardZone = zone->getParent();
    boardZone->removeOne(zone);
    delete zone;
    return true;
}

QList<const InnerDecklistNode *> DecklistNodeTree::getCustomZones(const QString &boardZoneName) const
{
    QList<const InnerDecklistNode *> result;

    auto *boardZone = findBoardZone(boardZoneName);
    if (!boardZone) {
        return result;
    }

    for (int i = 0; i < boardZone->size(); i++) {
        if (auto *customZone = dynamic_cast<InnerDecklistNode *>(boardZone->at(i))) {
            result.append(customZone);
        }
    }

    return result;
}

InnerDecklistNode *DecklistNodeTree::findCustomZoneByName(const QString &zoneName) const
{
    for (int i = 0; i < root->size(); i++) {
        auto *boardZone = dynamic_cast<InnerDecklistNode *>(root->at(i));
        if (!boardZone) {
            continue;
        }

        for (int j = 0; j < boardZone->size(); j++) {
            auto *customZone = dynamic_cast<InnerDecklistNode *>(boardZone->at(j));
            if (customZone && customZone->getName() == zoneName) {
                return customZone;
            }
        }
    }

    return nullptr;
}

bool DecklistNodeTree::hasZoneName(const QString &zoneName) const
{
    // The standard zones are reserved names even before they are created lazily.
    if (zoneName == DECK_ZONE_MAIN || zoneName == DECK_ZONE_SIDE || zoneName == DECK_ZONE_MAYBEBOARD ||
        zoneName == DECK_ZONE_TOKENS) {
        return true;
    }

    if (root->findChild(zoneName)) {
        return true;
    }

    return findCustomZoneByName(zoneName) != nullptr;
}
