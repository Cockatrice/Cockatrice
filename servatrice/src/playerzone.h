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
class Random;

class PlayerZone {
private:
	QString name;
	bool has_coords;
	bool is_public; // Contents of the zone are always visible to anyone
	bool is_private; // Contents of the zone are always visible to the owner
	bool id_access; // getCard() finds by id, not by list index
	// Example: When moving a card from the library to the table,
	// the card has to be found by list index because the client
	// does not know the id. But when moving a card from the hand
	// to the table, the card can be found by id because the client
	// knows the id and the hand does not need to have a specific order.
public:
	PlayerZone(QString _name, bool _has_coords, bool _is_public, bool _is_private, bool _id_access);
	~PlayerZone();

	Card *getCard(int id, bool remove, int *position = NULL);

	bool isPublic() { return is_public; }
	bool isPrivate() { return is_private; }
	bool hasCoords() { return has_coords; }
	bool hasIdAccess() { return id_access; }
	QString getName() { return name; }
	
	QList<Card *> cards;
	void insertCard(Card *card, int x, int y);
	void shuffle(Random *rnd);
	void clear();
};

#endif
