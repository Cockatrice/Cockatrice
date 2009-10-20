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
#ifndef PLAYERZONE_H
#define PLAYERZONE_H

#include <QList>
#include <QString>

class Card;
class ServerSocket;
class AbstractRNG;

class PlayerZone {
public:
	// PrivateZone: Contents of the zone are always visible to the owner,
	// but not to anyone else.
	// PublicZone: Contents of the zone are always visible to anyone.
	// HiddenZone: Contents of the zone are never visible to anyone.
	// However, the owner of the zone can issue a dump_zone command,
	// setting beingLookedAt to true.
	// Cards in a zone with the type HiddenZone are referenced by their
	// list index, whereas cards in any other zone are referenced by their ids.
	enum ZoneType { PrivateZone, PublicZone, HiddenZone };
private:
	ServerSocket *player;
	QString name;
	bool has_coords;
	ZoneType type;
	int cardsBeingLookedAt;
public:
	PlayerZone(ServerSocket *_player, const QString &_name, bool _has_coords, ZoneType _type);
	~PlayerZone();

	Card *getCard(int id, bool remove, int *position = NULL);

	int getCardsBeingLookedAt() const { return cardsBeingLookedAt; }
	void setCardsBeingLookedAt(int _cardsBeingLookedAt) { cardsBeingLookedAt = _cardsBeingLookedAt; }
	bool hasCoords() const { return has_coords; }
	ZoneType getType() const { return type; }
	QString getName() const { return name; }
	ServerSocket *getPlayer() const { return player; }
	
	QList<Card *> cards;
	void insertCard(Card *card, int x, int y);
	void shuffle(AbstractRNG *rnd);
	void clear();
};

#endif
