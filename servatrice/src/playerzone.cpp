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
#include "playerzone.h"

PlayerZone::PlayerZone(QString _name, bool _has_coords, bool _is_public, bool _is_private, bool _id_access)
	: name(_name), has_coords(_has_coords), is_public(_is_public), is_private(_is_private), id_access(_id_access)
{
}

PlayerZone::~PlayerZone()
{
	qDebug(QString("PlayerZone destructor: %1").arg(name).toLatin1());
	clear();
}

void PlayerZone::shuffle(TestRandom *rnd)
{
	QList<TestCard *> temp;
	for (int i = cards.size(); i; i--)
		temp.append(cards.takeAt(rnd->getNumber(0, i - 1)));
	cards = temp;
}

TestCard *PlayerZone::getCard(int id, bool remove, int *position)
{
	if (hasIdAccess()) {
		QListIterator<TestCard *> CardIterator(cards);
		int i = 0;
		while (CardIterator.hasNext()) {
			TestCard *tmp = CardIterator.next();
			if (tmp->getId() == id) {
				if (remove)
					cards.removeAt(i);
				if (position)
					*position = i;
				return tmp;
			}
			i++;
		}
		return NULL;
	} else {
		if (id >= cards.size())
			return NULL;
		TestCard *tmp = cards[id];
		if (remove)
			cards.removeAt(id);
		if (position)
			*position = id;
		return tmp;
	}
}

void PlayerZone::insertCard(TestCard *card, int x, int y)
{
	if (hasCoords()) {
		card->setCoords(x, y);
		cards.append(card);
	} else {
		card->setCoords(0, 0);
		cards.insert(x, card);
	}
}

void PlayerZone::clear()
{
	for (int i = 0; i < cards.size(); i++)
		delete cards.at(i);
	cards.clear();
}
