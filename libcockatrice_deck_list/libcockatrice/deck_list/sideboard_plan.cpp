#include "sideboard_plan.h"

#include <QXmlStreamReader>

namespace
{

void readMoveCardToZone(QXmlStreamReader *xml, QList<MoveCard_ToZone> &moveList)
{
    MoveCard_ToZone move;
    while (!xml->atEnd()) {
        xml->readNext();
        const QString childName = xml->name().toString();
        if (xml->isStartElement()) {
            if (childName == "card_name") {
                move.set_card_name(xml->readElementText().toStdString());
            } else if (childName == "start_zone") {
                move.set_start_zone(xml->readElementText().toStdString());
            } else if (childName == "target_zone") {
                move.set_target_zone(xml->readElementText().toStdString());
            }
        } else if (xml->isEndElement() && (childName == "move_card_to_zone")) {
            moveList.append(move);
            return;
        }
    }
}

} // namespace

SideboardPlan::SideboardPlan(const QString &_name, const QList<MoveCard_ToZone> &_moveList)
    : name(_name), moveList(_moveList)
{
}

void SideboardPlan::setMoveList(const QList<MoveCard_ToZone> &_moveList)
{
    moveList = _moveList;
}

bool SideboardPlan::readElement(QXmlStreamReader *xml)
{
    while (!xml->atEnd()) {
        xml->readNext();
        const QString childName = xml->name().toString();
        if (xml->isStartElement()) {
            if (childName == "name") {
                name = xml->readElementText();
            } else if (childName == "move_card_to_zone") {
                readMoveCardToZone(xml, moveList);
            }
        } else if (xml->isEndElement() && (childName == "sideboard_plan")) {
            return true;
        }
    }
    return false;
}

void SideboardPlan::write(QXmlStreamWriter *xml) const
{
    xml->writeStartElement("sideboard_plan");
    xml->writeTextElement("name", name);
    for (auto &i : moveList) {
        xml->writeStartElement("move_card_to_zone");
        xml->writeTextElement("card_name", QString::fromStdString(i.card_name()));
        xml->writeTextElement("start_zone", QString::fromStdString(i.start_zone()));
        xml->writeTextElement("target_zone", QString::fromStdString(i.target_zone()));
        xml->writeEndElement();
    }
    xml->writeEndElement();
}