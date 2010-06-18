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
#ifndef SERVER_CARDZONE_H
#define SERVER_CARDZONE_H

#include <QList>
#include <QString>
#include "protocol_datastructures.h"

class Server_Card;
class Server_Player;

class Server_CardZone {
private:
	Server_Player *player;
	QString name;
	bool has_coords;
	ZoneType type;
	int cardsBeingLookedAt;
public:
	Server_CardZone(Server_Player *_player, const QString &_name, bool _has_coords, ZoneType _type);
	~Server_CardZone();

	Server_Card *getCard(int id, bool remove, int *position = NULL);

	int getCardsBeingLookedAt() const { return cardsBeingLookedAt; }
	void setCardsBeingLookedAt(int _cardsBeingLookedAt) { cardsBeingLookedAt = _cardsBeingLookedAt; }
	bool hasCoords() const { return has_coords; }
	ZoneType getType() const { return type; }
	QString getName() const { return name; }
	Server_Player *getPlayer() const { return player; }
	
	int getFreeGridColumn(int y) const;
	QList<Server_Card *> cards;
	void insertCard(Server_Card *card, int x, int y);
	void shuffle();
	void clear();
};

#endif
