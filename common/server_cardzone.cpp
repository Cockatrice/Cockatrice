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
#include "server_cardzone.h"
#include "server_card.h"
#include "rng_abstract.h"

Server_CardZone::Server_CardZone(Server_Player *_player, const QString &_name, bool _has_coords, ZoneType _type)
	: player(_player), name(_name), has_coords(_has_coords), type(_type), cardsBeingLookedAt(0)
{
}

Server_CardZone::~Server_CardZone()
{
	qDebug(QString("Server_CardZone destructor: %1").arg(name).toLatin1());
	clear();
}

void Server_CardZone::shuffle()
{
	QList<Server_Card *> temp;
	for (int i = cards.size(); i; i--)
		temp.append(cards.takeAt(rng->getNumber(0, i - 1)));
	cards = temp;
}

Server_Card *Server_CardZone::getCard(int id, bool remove, int *position)
{
	if (type != HiddenZone) {
		QListIterator<Server_Card *> CardIterator(cards);
		int i = 0;
		while (CardIterator.hasNext()) {
			Server_Card *tmp = CardIterator.next();
			if (tmp->getId() == id) {
				if (remove) {
					cards.removeAt(i);
					tmp->setZone(0);
				}
				if (position)
					*position = i;
				return tmp;
			}
			i++;
		}
		return NULL;
	} else {
		if ((id >= cards.size()) || (id < 0))
			return NULL;
		Server_Card *tmp = cards[id];
		if (remove) {
			cards.removeAt(id);
			tmp->setZone(0);
		}
		if (position)
			*position = id;
		return tmp;
	}
}

void Server_CardZone::insertCard(Server_Card *card, int x, int y)
{
	if (hasCoords()) {
		card->setCoords(x, y);
		cards.append(card);
	} else {
		card->setCoords(0, 0);
		cards.insert(x, card);
	}
	card->setZone(this);
}

void Server_CardZone::clear()
{
	for (int i = 0; i < cards.size(); i++)
		delete cards.at(i);
	cards.clear();
}
