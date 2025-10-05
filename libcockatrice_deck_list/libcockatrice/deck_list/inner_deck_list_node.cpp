#include "inner_deck_list_node.h"

#include "deck_list_card_node.h"

InnerDecklistNode::InnerDecklistNode(InnerDecklistNode *other, InnerDecklistNode *_parent)
    : AbstractDecklistNode(_parent), name(other->getName())
{
    for (int i = 0; i < other->size(); ++i) {
        auto *inner = dynamic_cast<InnerDecklistNode *>(other->at(i));
        if (inner) {
            new InnerDecklistNode(inner, this);
        } else {
            new DecklistCardNode(dynamic_cast<DecklistCardNode *>(other->at(i)), this);
        }
    }
}

InnerDecklistNode::~InnerDecklistNode()
{
    clearTree();
}

QString InnerDecklistNode::visibleNameFromName(const QString &_name)
{
    if (_name == DECK_ZONE_MAIN) {
        return QObject::tr("Maindeck");
    } else if (_name == DECK_ZONE_SIDE) {
        return QObject::tr("Sideboard");
    } else if (_name == DECK_ZONE_TOKENS) {
        return QObject::tr("Tokens");
    } else {
        return _name;
    }
}

void InnerDecklistNode::setSortMethod(DeckSortMethod method)
{
    sortMethod = method;
    for (int i = 0; i < size(); i++) {
        at(i)->setSortMethod(method);
    }
}

QString InnerDecklistNode::getVisibleName() const
{
    return visibleNameFromName(name);
}

void InnerDecklistNode::clearTree()
{
    for (int i = 0; i < size(); i++)
        delete at(i);
    clear();
}

AbstractDecklistNode *InnerDecklistNode::findChild(const QString &_name)
{
    for (int i = 0; i < size(); i++) {
        if (at(i)->getName() == _name) {
            return at(i);
        }
    }
    return nullptr;
}

AbstractDecklistNode *InnerDecklistNode::findCardChildByNameProviderIdAndNumber(const QString &_name,
                                                                                const QString &_providerId,
                                                                                const QString &_cardNumber)
{
    for (const auto &i : *this) {
        if (!i || i->getName() != _name) {
            continue;
        }
        if (_cardNumber != "" && i->getCardCollectorNumber() != _cardNumber) {
            continue;
        }
        if (_providerId != "" && i->getCardProviderId() != _providerId) {
            continue;
        }
        return i;
    }
    return nullptr;
}

int InnerDecklistNode::height() const
{
    return at(0)->height() + 1;
}

int InnerDecklistNode::recursiveCount(bool countTotalCards) const
{
    int result = 0;
    for (int i = 0; i < size(); i++) {
        auto *node = dynamic_cast<InnerDecklistNode *>(at(i));

        if (node) {
            result += node->recursiveCount(countTotalCards);
        } else if (countTotalCards) {
            result += dynamic_cast<AbstractDecklistCardNode *>(at(i))->getNumber();
        } else {
            result++;
        }
    }
    return result;
}

bool InnerDecklistNode::compare(AbstractDecklistNode *other) const
{
    switch (sortMethod) {
        case ByNumber:
            return compareNumber(other);
        case ByName:
            return compareName(other);
        default:
            return false;
    }
}

bool InnerDecklistNode::compareNumber(AbstractDecklistNode *other) const
{
    auto *other2 = dynamic_cast<InnerDecklistNode *>(other);
    if (other2) {
        int n1 = recursiveCount(true);
        int n2 = other2->recursiveCount(true);
        return (n1 != n2) ? (n1 > n2) : compareName(other);
    } else {
        return false;
    }
}

bool InnerDecklistNode::compareName(AbstractDecklistNode *other) const
{
    auto *other2 = dynamic_cast<InnerDecklistNode *>(other);
    if (other2) {
        return (getName() > other2->getName());
    } else {
        return false;
    }
}

bool InnerDecklistNode::readElement(QXmlStreamReader *xml)
{
    while (!xml->atEnd()) {
        xml->readNext();
        const QString childName = xml->name().toString();
        if (xml->isStartElement()) {
            if (childName == "zone") {
                InnerDecklistNode *newZone = new InnerDecklistNode(xml->attributes().value("name").toString(), this);
                newZone->readElement(xml);
            } else if (childName == "card") {
                DecklistCardNode *newCard = new DecklistCardNode(
                    xml->attributes().value("name").toString(), xml->attributes().value("number").toString().toInt(),
                    this, -1, xml->attributes().value("setShortName").toString(),
                    xml->attributes().value("collectorNumber").toString(), xml->attributes().value("uuid").toString());
                newCard->readElement(xml);
            }
        } else if (xml->isEndElement() && (childName == "zone"))
            return false;
    }
    return true;
}

void InnerDecklistNode::writeElement(QXmlStreamWriter *xml)
{
    xml->writeStartElement("zone");
    xml->writeAttribute("name", name);
    for (int i = 0; i < size(); i++)
        at(i)->writeElement(xml);
    xml->writeEndElement(); // zone
}

QVector<QPair<int, int>> InnerDecklistNode::sort(Qt::SortOrder order)
{
    QVector<QPair<int, int>> result(size());

    // Initialize temporary list with contents of current list
    QVector<QPair<int, AbstractDecklistNode *>> tempList(size());
    for (int i = size() - 1; i >= 0; --i) {
        tempList[i].first = i;
        tempList[i].second = at(i);
    }

    // Sort temporary list
    auto cmp = [order](const auto &a, const auto &b) {
        return (order == Qt::AscendingOrder) ? (b.second->compare(a.second)) : (a.second->compare(b.second));
    };

    std::sort(tempList.begin(), tempList.end(), cmp);

    // Map old indexes to new indexes and
    // copy temporary list to the current one
    for (int i = size() - 1; i >= 0; --i) {
        result[i].first = tempList[i].first;
        result[i].second = i;
        replace(i, tempList[i].second);
    }

    return result;
}