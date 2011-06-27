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

Server_Card::Server_Card(QString _name, int _id, int _coord_x, int _coord_y)
	: id(_id), coord_x(_coord_x), coord_y(_coord_y), name(_name), tapped(false), attacking(false), facedown(false), color(QString()), power(-1), toughness(-1), annotation(QString()), destroyOnZoneChange(false), doesntUntap(false), parentCard(0)
{
}

Server_Card::~Server_Card()
{
	// setParentCard(0) leads to the item being removed from our list, so we can't iterate properly
	while (!attachedCards.isEmpty())
		attachedCards.first()->setParentCard(0);
	
	if (parentCard)
		parentCard->removeAttachedCard(this);
}

void Server_Card::resetState()
{
	counters.clear();
	setTapped(false);
	setAttacking(false);
	power = -1;
	toughness = -1;
	setAnnotation(QString());
	setDoesntUntap(false);
}

QString Server_Card::setAttribute(const QString &aname, const QString &avalue, bool allCards)
{
	if (aname == "tapped") {
		bool value = avalue == "1";
		if (!(!value && allCards && doesntUntap))
			setTapped(value);
	} else if (aname == "attacking") {
		setAttacking(avalue == "1");
	} else if (aname == "facedown") {
		setFaceDown(avalue == "1");
	} else if (aname == "color") {
		setColor(avalue);
	} else if (aname == "pt") {
		setPT(avalue);
		return getPT();
	} else if (aname == "annotation") {
		setAnnotation(avalue);
	} else if (aname == "doesnt_untap") {
		setDoesntUntap(avalue == "1");
	} else
		return QString();
	
	return avalue;
}

void Server_Card::setCounter(int id, int value)
{
	if (value)
		counters.insert(id, value);
	else
		counters.remove(id);
}

void Server_Card::setPT(const QString &_pt)
{
	if (_pt.isEmpty()) {
		power = 0;
		toughness = -1;
	} else {
		int sep = _pt.indexOf('/');
		QString p1 = _pt.left(sep);
		QString p2 = _pt.mid(sep + 1);
		if (p1.isEmpty() || p2.isEmpty())
			return;
		if ((p1[0] == '+') || (p2[0] == '+')) {
			if (power < 0)
				power = 0;
			if (toughness < 0)
				toughness = 0;
		}
		if (p1[0] == '+')
			power += p1.mid(1).toInt();
		else
			power = p1.toInt();
		if (p2[0] == '+')
			toughness += p2.mid(1).toInt();
		else
			toughness = p2.toInt();
	}
}

QString Server_Card::getPT() const
{
	if (toughness < 0)
		return QString("");
	return QString::number(power) + "/" + QString::number(toughness);
}

void Server_Card::setParentCard(Server_Card *_parentCard)
{
	if (parentCard)
		parentCard->removeAttachedCard(this);
	parentCard = _parentCard;
	if (parentCard)
		parentCard->addAttachedCard(this);
}
