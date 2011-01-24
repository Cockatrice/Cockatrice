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
#include "server_player.h"
#include "rng_abstract.h"
#include <QSet>
#include <QDebug>

Server_CardZone::Server_CardZone(Server_Player *_player, const QString &_name, bool _has_coords, ZoneType _type)
	: player(_player), name(_name), has_coords(_has_coords), type(_type), cardsBeingLookedAt(0)
{
}

Server_CardZone::~Server_CardZone()
{
	qDebug() << "Server_CardZone destructor:" << name;
	clear();
}

void Server_CardZone::shuffle()
{
	QList<Server_Card *> temp;
	for (int i = cards.size(); i; i--)
		temp.append(cards.takeAt(rng->getNumber(0, i - 1)));
	cards = temp;
}

int Server_CardZone::removeCard(Server_Card *card)
{
	int index = cards.indexOf(card);
	cards.removeAt(index);
	return index;
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

int Server_CardZone::getFreeGridColumn(int x, int y, const QString &cardName) const
{
	QMap<int, Server_Card *> coordMap;
	for (int i = 0; i < cards.size(); ++i)
		if (cards[i]->getY() == y)
			coordMap.insert(cards[i]->getX(), cards[i]);
	
	int resultX = 0;
	if (x == -1) {
		for (int i = 0; i < cards.size(); ++i)
			if ((cards[i]->getName() == cardName) && !(cards[i]->getX() % 3) && (cards[i]->getY() == y)) {
				if (!cards[i]->getAttachedCards().isEmpty())
					continue;
				if (!coordMap.value(cards[i]->getX() + 1))
					return cards[i]->getX() + 1;
				if (!coordMap.value(cards[i]->getX() + 2))
					return cards[i]->getX() + 2;
			}
	} else if (x == -2) {
	} else {
		x = (x / 3) * 3;
		if (!coordMap.contains(x))
			resultX = x;
		else if (!coordMap.value(x)->getAttachedCards().isEmpty()) {
			resultX = x;
			x = -1;
		} else if (!coordMap.contains(x + 1))
			resultX = x + 1;
		else if (!coordMap.contains(x + 2))
			resultX = x + 2;
		else {
			resultX = x;
			x = -1;
		}
	}
	
	if (x < 0)
		while (coordMap.value(resultX))
			resultX += 3;

	return resultX;
}

bool Server_CardZone::isColumnStacked(int x, int y) const
{
	if (!has_coords)
		return false;
	
	QMap<int, Server_Card *> coordMap;
	for (int i = 0; i < cards.size(); ++i)
		if (cards[i]->getY() == y)
			coordMap.insert(cards[i]->getX(), cards[i]);
	
	return coordMap.contains((x / 3) * 3 + 1);
}

bool Server_CardZone::isColumnEmpty(int x, int y) const
{
	if (!has_coords)
		return true;
	
	QMap<int, Server_Card *> coordMap;
	for (int i = 0; i < cards.size(); ++i)
		if (cards[i]->getY() == y)
			coordMap.insert(cards[i]->getX(), cards[i]);
	
	return !coordMap.contains((x / 3) * 3);
}

void Server_CardZone::moveCard(CommandContainer *cont, QMap<int, Server_Card *> &coordMap, Server_Card *card, int x, int y)
{
	coordMap.remove(card->getY() * 10000 + card->getX());
	player->moveCard(cont, this, QList<int>() << card->getId(), this, x, y, card->getFaceDown(), false, false);
	coordMap.insert(y * 10000 + x, card);
}

void Server_CardZone::fixFreeSpaces(CommandContainer *cont)
{
	QMap<int, Server_Card *> coordMap;
	QSet<int> placesToLook;
	for (int i = 0; i < cards.size(); ++i) {
		coordMap.insert(cards[i]->getY() * 10000 + cards[i]->getX(), cards[i]);
		placesToLook.insert(cards[i]->getY() * 10000 + (cards[i]->getX() / 3) * 3);
	}
	
	QSetIterator<int> placeIterator(placesToLook);
	while (placeIterator.hasNext()) {
		int foo = placeIterator.next();
		int y = foo / 10000;
		int baseX = foo - y * 10000;
		
		if (!coordMap.contains(y * 10000 + baseX)) {
			if (coordMap.contains(y * 10000 + baseX + 1))
				moveCard(cont, coordMap, coordMap.value(y * 10000 + baseX + 1), baseX, y);
			else if (coordMap.contains(baseX + 2)) {
				moveCard(cont, coordMap, coordMap.value(y * 10000 + baseX + 2), baseX, y);
				return;
			} else
				return;
		}
		if (!coordMap.contains(y * 10000 + baseX + 1) && coordMap.contains(y * 10000 + baseX + 2))
			moveCard(cont, coordMap, coordMap.value(y * 10000 + baseX + 2), baseX + 1, y);
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
