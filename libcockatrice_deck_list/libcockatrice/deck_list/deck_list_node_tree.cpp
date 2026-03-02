#include "deck_list_node_tree.h"

#include "tree/deck_list_card_node.h"

#include <QCryptographicHash>
#include <QSet>

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
        for (auto *cardNode : *zoneNode) {
            auto *cardCardNode = dynamic_cast<DecklistCardNode *>(cardNode);
            if (cardCardNode) {
                result.append(cardCardNode);
            }
        }
    }

    return result;
}

QList<const InnerDecklistNode *> DecklistNodeTree::getZoneNodes(const QSet<QString> &restrictToZones) const
{
    QList<const InnerDecklistNode *> zones;
    for (auto *node : *root) {
        InnerDecklistNode *currentZone = dynamic_cast<InnerDecklistNode *>(node);
        if (!currentZone)
            continue;
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

    static auto nodesToCardList = [](const QList<const DecklistCardNode *> &nodes, const QString &prefix = {}) {
        QStringList result;
        for (auto node : nodes) {
            for (int i = 0; i < node->getNumber(); ++i) {
                result.append(prefix + node->getName().toLower());
            }
        }
        return result;
    };

    QStringList cardList = nodesToCardList(mainDeckNodes) + nodesToCardList(sideDeckNodes, "SB:");

    cardList.sort();
    QByteArray deckHashArray = QCryptographicHash::hash(cardList.join(";").toUtf8(), QCryptographicHash::Sha1);
    quint64 number = (((quint64)(unsigned char)deckHashArray[0]) << 32) +
                     (((quint64)(unsigned char)deckHashArray[1]) << 24) +
                     (((quint64)(unsigned char)deckHashArray[2] << 16)) +
                     (((quint64)(unsigned char)deckHashArray[3]) << 8) + (quint64)(unsigned char)deckHashArray[4];
    return QString::number(number, 32).rightJustified(8, '0');
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
    newZone->readElement(xml);
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

        if (rootNode->empty()) {
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
    // Support for this is only possible if the internal structure
    // doesn't get more complicated.
    for (int i = 0; i < root->size(); i++) {
        InnerDecklistNode *node = dynamic_cast<InnerDecklistNode *>(root->at(i));
        for (int j = 0; j < node->size(); j++) {
            DecklistCardNode *card = dynamic_cast<DecklistCardNode *>(node->at(j));
            func(node, card);
        }
    }
}

/**
 * Gets the InnerDecklistNode that is the root node for the given zone, creating a new node if it doesn't exist.
 */
InnerDecklistNode *DecklistNodeTree::getZoneObjFromName(const QString &zoneName) const
{
    for (int i = 0; i < root->size(); i++) {
        auto *node = dynamic_cast<InnerDecklistNode *>(root->at(i));
        if (node->getName() == zoneName) {
            return node;
        }
    }

    return new InnerDecklistNode(zoneName, root);
}
