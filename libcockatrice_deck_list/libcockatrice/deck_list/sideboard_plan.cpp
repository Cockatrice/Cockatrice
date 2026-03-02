#include "sideboard_plan.h"

#include <QXmlStreamReader>

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
            if (childName == "name")
                name = xml->readElementText();
            else if (childName == "move_card_to_zone") {
                MoveCard_ToZone m;
                while (!xml->atEnd()) {
                    xml->readNext();
                    const QString childName2 = xml->name().toString();
                    if (xml->isStartElement()) {
                        if (childName2 == "card_name")
                            m.set_card_name(xml->readElementText().toStdString());
                        else if (childName2 == "start_zone")
                            m.set_start_zone(xml->readElementText().toStdString());
                        else if (childName2 == "target_zone")
                            m.set_target_zone(xml->readElementText().toStdString());
                    } else if (xml->isEndElement() && (childName2 == "move_card_to_zone")) {
                        moveList.append(m);
                        break;
                    }
                }
            }
        } else if (xml->isEndElement() && (childName == "sideboard_plan"))
            return true;
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