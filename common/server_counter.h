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
#ifndef SERVER_COUNTER_H
#define SERVER_COUNTER_H

#include <QString>
#include "pb/color.pb.h"

class ServerInfo_Counter;

class Server_Counter {
protected:
	int id;
	QString name;
	color counterColor;
	int radius;
	int count;
public:
	Server_Counter(int _id, const QString &_name, const color &_counterColor, int _radius, int _count = 0);
	~Server_Counter() { }
	int getId() const { return id; }
	QString getName() const { return name; }
	const color &getColor() const { return counterColor; }
	int getRadius() const { return radius; }
	int getCount() const { return count; }
	void setCount(int _count) { count = _count; }
	
	void getInfo(ServerInfo_Counter *info);
};

#endif
