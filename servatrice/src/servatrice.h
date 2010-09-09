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
#ifndef SERVATRICE_H
#define SERVATRICE_H

#include <QTcpServer>
#include "server.h"

class QSqlDatabase;
class QSettings;
class QSqlQuery;
class QTimer;

class Servatrice : public Server
{
	Q_OBJECT
private slots:
	void newConnection();
public:
	static const QString versionString;
	Servatrice(QObject *parent = 0);
	~Servatrice();
	bool openDatabase();
	void checkSql();
	bool execSqlQuery(QSqlQuery &query);
	AuthenticationResult checkUserPassword(const QString &user, const QString &password);
	QString getLoginMessage() const { return loginMessage; }
	bool getGameShouldPing() const { return true; }
	int getMaxGameInactivityTime() const { return maxGameInactivityTime; }
	int getMaxPlayerInactivityTime() const { return maxPlayerInactivityTime; }
private:
	QTimer *pingClock;
	QTcpServer *tcpServer;
	QString loginMessage;
	QSettings *settings;
	int maxGameInactivityTime;
	int maxPlayerInactivityTime;
};

#endif
