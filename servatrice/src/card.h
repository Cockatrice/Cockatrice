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
#ifndef CARD_H
#define CARD_H

#include <QString>

class Card {
private:
	int id;
	int coord_x, coord_y;
	QString name;
	int counters;
	bool tapped;
	bool attacking;
	bool facedown;
	QString annotation;
	bool doesntUntap;
public:
	Card(QString _name, int _id, int _coord_x, int _coord_y);
	~Card();
	
	int getId() const { return id; }
	int getX() const { return coord_x; }
	int getY() const { return coord_y; }
	QString getName() const { return name; }
	int getCounters() const { return counters; }
	bool getTapped() const { return tapped; }
	bool getAttacking() const { return attacking; }
	bool getFaceDown() const { return facedown; }
	QString getAnnotation() const { return annotation; }
	bool getDoesntUntap() const { return doesntUntap; }

	void setId(int _id) { id = _id; }
	void setCoords(int x, int y) { coord_x = x; coord_y = y; }
	void setName(const QString &_name) { name = _name; }
	void setCounters(int _counters) { counters = _counters; }
	void setTapped(bool _tapped) { tapped = _tapped; }
	void setAttacking(bool _attacking) { attacking = _attacking; }
	void setFaceDown(bool _facedown) { facedown = _facedown; }
	void setAnnotation(const QString &_annotation) { annotation = _annotation; }
	void setDoesntUntap(bool _doesntUntap) { doesntUntap = _doesntUntap; }
	
	void resetState();
	bool setAttribute(const QString &aname, const QString &avalue, bool allCards);
};

#endif
