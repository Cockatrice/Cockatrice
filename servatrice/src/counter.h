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
#ifndef COUNTER_H
#define COUNTER_H

#include <QString>

class Counter {
protected:
	int id;
	QString name;
	int color;
	int radius;
	int count;
public:
	Counter(int _id, const QString &_name, int _color, int _radius, int _count = 0) : id(_id), name(_name), color(_color), radius(_radius), count(_count) { }
	~Counter() { }
	int getId() const { return id; }
	QString getName() const { return name; }
	int getColor() const { return color; }
	int getRadius() const { return radius; }
	int getCount() const { return count; }
	void setCount(int _count) { count = _count; }
};

#endif
