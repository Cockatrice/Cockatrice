/***************************************************************************
 *   Copyright (C) 2008 by Max-Wilhelm Bruker   *
 *   brukie@laptop   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "server_card.h"

#include "pb/event_set_card_attr.pb.h"
#include "pb/event_set_card_counter.pb.h"
#include "pb/serverinfo_card.pb.h"
#include "server_cardzone.h"
#include "server_player.h"

#include <QVariant>

Server_Card::Server_Card(QString _name, int _id, int _coord_x, int _coord_y, Server_CardZone *_zone)
    : zone(_zone), id(_id), coord_x(_coord_x), coord_y(_coord_y), name(_name), tapped(false), attacking(false),
      facedown(false), color(), ptString(), annotation(), destroyOnZoneChange(false), doesntUntap(false), parentCard(0),
      stashedCard(nullptr)
{
}

Server_Card::~Server_Card()
{
    // setParentCard(0) leads to the item being removed from our list, so we can't iterate properly
    while (!attachedCards.isEmpty())
        attachedCards.first()->setParentCard(0);

    if (parentCard)
        parentCard->removeAttachedCard(this);

    if (stashedCard) {
        stashedCard->deleteLater();
        stashedCard = nullptr;
    }
}

void Server_Card::resetState()
{
    counters.clear();
    setTapped(false);
    setAttacking(false);
    setPT(QString());
    setAnnotation(QString());
    setDoesntUntap(false);
}

QString Server_Card::setAttribute(CardAttribute attribute, const QString &avalue, bool allCards)
{
    if (attribute == AttrTapped && avalue != "1" && allCards && doesntUntap)
        return QVariant(tapped).toString();

    return setAttribute(attribute, avalue);
}

QString Server_Card::setAttribute(CardAttribute attribute, const QString &avalue, Event_SetCardAttr *event)
{
    if (event)
        event->set_attribute(attribute);

    switch (attribute) {
        case AttrTapped: {
            setTapped(avalue == "1");
            break;
        }
        case AttrAttacking:
            setAttacking(avalue == "1");
            break;
        case AttrFaceDown:
            setFaceDown(avalue == "1");
            break;
        case AttrColor:
            setColor(avalue);
            break;
        case AttrPT:
            setPT(avalue);
            if (event)
                event->set_attr_value(getPT().toStdString());
            return getPT();
        case AttrAnnotation:
            setAnnotation(avalue);
            break;
        case AttrDoesntUntap:
            setDoesntUntap(avalue == "1");
            break;
    }
    if (event)
        event->set_attr_value(avalue.toStdString());
    return avalue;
}

void Server_Card::setCounter(int _id, int value, Event_SetCardCounter *event)
{
    if (value)
        counters.insert(_id, value);
    else
        counters.remove(_id);

    if (event) {
        event->set_counter_id(_id);
        event->set_counter_value(value);
    }
}

void Server_Card::setParentCard(Server_Card *_parentCard)
{
    if (parentCard)
        parentCard->removeAttachedCard(this);
    parentCard = _parentCard;
    if (parentCard)
        parentCard->addAttachedCard(this);
}

void Server_Card::getInfo(ServerInfo_Card *info)
{
    QString displayedName = facedown ? QString() : name;

    info->set_id(id);
    info->set_name(displayedName.toStdString());
    info->set_x(coord_x);
    info->set_y(coord_y);
    if (facedown) {
        info->set_face_down(true);
    }
    info->set_tapped(tapped);
    if (attacking) {
        info->set_attacking(true);
    }
    if (!color.isEmpty()) {
        info->set_color(color.toStdString());
    }
    if (!ptString.isEmpty()) {
        info->set_pt(ptString.toStdString());
    }
    if (!annotation.isEmpty()) {
        info->set_annotation(annotation.toStdString());
    }
    if (destroyOnZoneChange) {
        info->set_destroy_on_zone_change(true);
    }
    if (doesntUntap) {
        info->set_doesnt_untap(true);
    }

    QMapIterator<int, int> cardCounterIterator(counters);
    while (cardCounterIterator.hasNext()) {
        cardCounterIterator.next();
        ServerInfo_CardCounter *counterInfo = info->add_counter_list();
        counterInfo->set_id(cardCounterIterator.key());
        counterInfo->set_value(cardCounterIterator.value());
    }

    if (parentCard) {
        info->set_attach_player_id(parentCard->getZone()->getPlayer()->getPlayerId());
        info->set_attach_zone(parentCard->getZone()->getName().toStdString());
        info->set_attach_card_id(parentCard->getId());
    }
}
