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
#include "pb/command_move_card.pb.h"

Server_CardZone::Server_CardZone(Server_Player *_player, const QString &_name, bool _has_coords, ServerInfo_Zone::ZoneType _type)
	: player(_player),
          name(_name),
          has_coords(_has_coords),
          type(_type),
          cardsBeingLookedAt(0),
          alwaysRevealTopCard(false)
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
	
	playersWithWritePermission.clear();
}

int Server_CardZone::removeCard(Server_Card *card)
{
	int index = cards.indexOf(card);
	cards.removeAt(index);
	if (has_coords)
		coordinateMap[card->getY()].remove(card->getX());
	card->setZone(0);
	
	return index;
}

Server_Card *Server_CardZone::getCard(int id, int *position, bool remove)
{
	if (type != ServerInfo_Zone::HiddenZone) {
		for (int i = 0; i < cards.size(); ++i) {
			Server_Card *tmp = cards[i];
			if (tmp->getId() == id) {
				if (position)
					*position = i;
				if (remove) {
					cards.removeAt(i);
					tmp->setZone(0);
				}
				return tmp;
			}
		}
		return NULL;
	} else {
		if ((id >= cards.size()) || (id < 0))
			return NULL;
		Server_Card *tmp = cards[id];
		if (position)
			*position = id;
		if (remove) {
			cards.removeAt(id);
			tmp->setZone(0);
		}
		return tmp;
	}
}

int Server_CardZone::getFreeGridColumn(int x, int y, const QString &cardName) const
{
	const QMap<int, Server_Card *> &coordMap = coordinateMap.value(y);
	int resultX = 0;
	if (x == -1) {
		QMapIterator<int, Server_Card *> cardIterator(coordMap);
		while (cardIterator.hasNext()) {
			cardIterator.next();
			const int cardX = cardIterator.key();
			if (cardX % 3)
				continue;
			Server_Card *card = cardIterator.value();
			if (card->getName() == cardName) {
				if (!card->getAttachedCards().isEmpty())
					continue;
				if (!coordMap.contains(cardX + 1))
					return cardX + 1;
				if (!coordMap.contains(cardX + 2))
					return cardX + 2;
			}
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
		while (coordMap.contains(resultX))
			resultX += 3;

	return resultX;
}

bool Server_CardZone::isColumnStacked(int x, int y) const
{
	if (!has_coords)
		return false;
	
	return coordinateMap[y].contains((x / 3) * 3 + 1);
}

bool Server_CardZone::isColumnEmpty(int x, int y) const
{
	if (!has_coords)
		return true;
	
	return !coordinateMap[y].contains((x / 3) * 3);
}

void Server_CardZone::moveCardInRow(GameEventStorage &ges, Server_Card *card, int x, int y)
{
	coordinateMap[y].remove(card->getX());
	
	CardToMove *cardToMove = new CardToMove;
	cardToMove->set_card_id(card->getId());
	player->moveCard(ges, this, QList<const CardToMove *>() << cardToMove, this, x, y, false, false);
	delete cardToMove;
	
	coordinateMap[y].insert(x, card);
}

void Server_CardZone::fixFreeSpaces(GameEventStorage &ges)
{
	if (!has_coords)
		return;
	
	QSet<QPair<int, int> > placesToLook;
	for (int i = 0; i < cards.size(); ++i)
		placesToLook.insert(QPair<int, int>((cards[i]->getX() / 3) * 3, cards[i]->getY()));
	
	QSetIterator<QPair<int, int> > placeIterator(placesToLook);
	while (placeIterator.hasNext()) {
		const QPair<int, int> &foo = placeIterator.next();
		int baseX = foo.first;
		int y = foo.second;
		
		if (!coordinateMap[y].contains(baseX)) {
			if (coordinateMap[y].contains(baseX + 1))
				moveCardInRow(ges, coordinateMap[y].value(baseX + 1), baseX, y);
			else if (coordinateMap[y].contains(baseX + 2)) {
				moveCardInRow(ges, coordinateMap[y].value(baseX + 2), baseX, y);
				continue;
			} else
				continue;
		}
		if (!coordinateMap[y].contains(baseX + 1) && coordinateMap[y].contains(baseX + 2))
			moveCardInRow(ges, coordinateMap[y].value(baseX + 2), baseX + 1, y);
	}
}

void Server_CardZone::updateCardCoordinates(Server_Card *card, int oldX, int oldY)
{
	if (!has_coords)
		return;
	
	if (oldX != -1)
		coordinateMap[oldY].remove(oldX);
	if (card->getX() != -1)
		coordinateMap[card->getY()].insert(card->getX(), card);
}

void Server_CardZone::insertCard(Server_Card *card, int x, int y)
{
	if (hasCoords()) {
		card->setCoords(x, y);
		cards.append(card);
		if (x != -1)
			coordinateMap[y].insert(x, card);
	} else {
		card->setCoords(0, 0);
		if (x == -1)
			cards.append(card);
		else
			cards.insert(x, card);
	}
	card->setZone(this);
}

void Server_CardZone::clear()
{
	for (int i = 0; i < cards.size(); i++)
		delete cards.at(i);
	cards.clear();
	coordinateMap.clear();
	playersWithWritePermission.clear();
}

void Server_CardZone::addWritePermission(int playerId)
{
	playersWithWritePermission.insert(playerId);
}

void Server_CardZone::getInfo(ServerInfo_Zone *info, Server_Player *playerWhosAsking, bool omniscient)
{
	info->set_name(name.toStdString());
	info->set_type(type);
	info->set_with_coords(has_coords);
	info->set_card_count(cards.size());
	info->set_always_reveal_top_card(alwaysRevealTopCard);
	if (
		(((playerWhosAsking == player) || omniscient) && (type != ServerInfo_Zone::HiddenZone))
		|| ((playerWhosAsking != player) && (type == ServerInfo_Zone::PublicZone))
	) {
		QListIterator<Server_Card *> cardIterator(cards);
		while (cardIterator.hasNext())
			cardIterator.next()->getInfo(info->add_card_list());
	}
}
