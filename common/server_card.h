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
#ifndef SERVER_CARD_H
#define SERVER_CARD_H

#include "server_arrowtarget.h"
#include "pb/card_attributes.pb.h"
#include <QString>
#include <QMap>

class Server_CardZone;
class ServerInfo_Card;

class Server_Card : public Server_ArrowTarget {
	Q_OBJECT
private:
	Server_CardZone *zone;
	int id;
	int coord_x, coord_y;
	QString name;
	QMap<int, int> counters;
	bool tapped;
	bool attacking;
	bool facedown;
	QString color;
	int power, toughness;
	QString annotation;
	bool destroyOnZoneChange;
	bool doesntUntap;
	
	Server_Card *parentCard;
	QList<Server_Card *> attachedCards;
public:
	Server_Card(QString _name, int _id, int _coord_x, int _coord_y, Server_CardZone *_zone = 0);
	~Server_Card();
	
	Server_CardZone *getZone() const { return zone; }
	void setZone(Server_CardZone *_zone) { zone = _zone; }
	
	int getId() const { return id; }
	int getX() const { return coord_x; }
	int getY() const { return coord_y; }
	QString getName() const { return name; }
	const QMap<int, int> &getCounters() const { return counters; }
	int getCounter(int id) const { return counters.value(id, 0); }
	bool getTapped() const { return tapped; }
	bool getAttacking() const { return attacking; }
	bool getFaceDown() const { return facedown; }
	QString getColor() const { return color; }
	QString getPT() const;
	QString getAnnotation() const { return annotation; }
	bool getDoesntUntap() const { return doesntUntap; }
	bool getDestroyOnZoneChange() const { return destroyOnZoneChange; }
	Server_Card *getParentCard() const { return parentCard; }
	const QList<Server_Card *> &getAttachedCards() const { return attachedCards; }

	void setId(int _id) { id = _id; }
	void setCoords(int x, int y) { coord_x = x; coord_y = y; }
	void setName(const QString &_name) { name = _name; }
	void setCounter(int id, int value);
	void setTapped(bool _tapped) { tapped = _tapped; }
	void setAttacking(bool _attacking) { attacking = _attacking; }
	void setFaceDown(bool _facedown) { facedown = _facedown; }
	void setColor(const QString &_color) { color = _color; }
	void setPT(const QString &_pt);
	void setAnnotation(const QString &_annotation) { annotation = _annotation; }
	void setDestroyOnZoneChange(bool _destroy) { destroyOnZoneChange = _destroy; }
	void setDoesntUntap(bool _doesntUntap) { doesntUntap = _doesntUntap; }
	void setParentCard(Server_Card *_parentCard);
	void addAttachedCard(Server_Card *card) { attachedCards.append(card); }
	void removeAttachedCard(Server_Card *card) { attachedCards.removeAt(attachedCards.indexOf(card)); }
	
	void resetState();
	QString setAttribute(CardAttribute attribute, const QString &avalue, bool allCards);
	
	void getInfo(ServerInfo_Card *info);
};

#endif
