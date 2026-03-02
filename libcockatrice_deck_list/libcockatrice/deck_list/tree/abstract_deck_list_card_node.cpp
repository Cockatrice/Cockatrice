#include "abstract_deck_list_card_node.h"

bool AbstractDecklistCardNode::compare(AbstractDecklistNode *other) const
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

bool AbstractDecklistCardNode::compareNumber(AbstractDecklistNode *other) const
{
    auto *other2 = dynamic_cast<AbstractDecklistCardNode *>(other);
    if (other2) {
        int n1 = getNumber();
        int n2 = other2->getNumber();
        return (n1 != n2) ? (n1 > n2) : compareName(other);
    } else {
        return true;
    }
}

bool AbstractDecklistCardNode::compareName(AbstractDecklistNode *other) const
{
    auto *other2 = dynamic_cast<AbstractDecklistCardNode *>(other);
    if (other2) {
        return (getName() > other2->getName());
    } else {
        return true;
    }
}

bool AbstractDecklistCardNode::readElement(QXmlStreamReader *xml)
{
    while (!xml->atEnd()) {
        xml->readNext();
        if (xml->isEndElement() && xml->name().toString() == "card")
            return false;
    }
    return true;
}

void AbstractDecklistCardNode::writeElement(QXmlStreamWriter *xml)
{
    xml->writeEmptyElement("card");
    xml->writeAttribute("number", QString::number(getNumber()));
    xml->writeAttribute("name", getName());

    if (!getCardSetShortName().isEmpty()) {
        xml->writeAttribute("setShortName", getCardSetShortName());
    }
    if (!getCardCollectorNumber().isEmpty()) {
        xml->writeAttribute("collectorNumber", getCardCollectorNumber());
    }
    if (!getCardProviderId().isEmpty()) {
        xml->writeAttribute("uuid", getCardProviderId());
    }
}