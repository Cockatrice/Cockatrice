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
#include <QtSql>
#include <QSettings>
#include "servatrice.h"
#include "server_chatchannel.h"
#include "serversocketinterface.h"
#include "protocol.h"

Servatrice::Servatrice(QObject *parent)
	: Server(parent)
{
	pingClock = new QTimer(this);
	connect(pingClock, SIGNAL(timeout()), this, SIGNAL(pingClockTimeout()));
	pingClock->start(1000);
	
	ProtocolItem::initializeHash();
	settings = new QSettings("servatrice.ini", QSettings::IniFormat, this);
	
	tcpServer = new QTcpServer(this);
	connect(tcpServer, SIGNAL(newConnection()), this, SLOT(newConnection()));
	tcpServer->listen(QHostAddress::Any, settings->value("server/port", 4747).toInt());
	
	QString dbType = settings->value("database/type").toString();
	if (dbType == "mysql")
		openDatabase();
	
	int size = settings->beginReadArray("chatchannels");
	for (int i = 0; i < size; ++i) {
	  	settings->setArrayIndex(i);
		Server_ChatChannel *newChannel = new Server_ChatChannel(
			settings->value("name").toString(),
			settings->value("description").toString(),
			settings->value("autojoin").toBool(),
			settings->value("joinmessage").toString()
		);
		addChatChannel(newChannel);
	}
	settings->endArray();
	
	loginMessage = settings->value("messages/login").toString();
	
	maxGameInactivityTime = settings->value("game/max_game_inactivity_time").toInt();
	maxPlayerInactivityTime = settings->value("game/max_player_inactivity_time").toInt();
}

Servatrice::~Servatrice()
{
}

bool Servatrice::openDatabase()
{
	if (!QSqlDatabase::connectionNames().isEmpty())
		QSqlDatabase::removeDatabase(QSqlDatabase::database().connectionNames().at(0));
	
	settings->beginGroup("database");
	QSqlDatabase sqldb = QSqlDatabase::addDatabase("QMYSQL");
	sqldb.setHostName(settings->value("hostname").toString());
	sqldb.setDatabaseName(settings->value("database").toString());
	sqldb.setUserName(settings->value("user").toString());
	sqldb.setPassword(settings->value("password").toString());
	settings->endGroup();
	
	if (!sqldb.open())
		return false;
	
	if (!nextGameId) {
		QSqlQuery query;
		if (!query.exec("select max(id) from games"))
			return false;
		if (!query.next())
			return false;
		nextGameId = query.value(0).toInt() + 1;
		qDebug(QString("set nextGameId to %1").arg(nextGameId).toLatin1());
	}
	return true;
}

void Servatrice::checkSql()
{
	if (!QSqlDatabase::database().exec("select 1").isActive())
		openDatabase();
}

bool Servatrice::execSqlQuery(QSqlQuery &query)
{
	if (query.exec())
		return true;
	qCritical(QString("Database error: %1").arg(query.lastError().text()).toLatin1());
	return false;
}

void Servatrice::newConnection()
{
	QTcpSocket *socket = tcpServer->nextPendingConnection();
	ServerSocketInterface *ssi = new ServerSocketInterface(this, socket);
	addClient(ssi);
}

AuthenticationResult Servatrice::checkUserPassword(const QString &user, const QString &password)
{
	const QString method = settings->value("authentication/method").toString();
	if (method == "none")
		return UnknownUser;
	else if (method == "sql") {
		checkSql();
	
		QSqlQuery query;
		query.prepare("select password from users where name = :name");
		query.bindValue(":name", user);
		if (!execSqlQuery(query))
			return PasswordWrong;
		
		if (query.next()) {
			if (query.value(0).toString() == password)
				return PasswordRight;
			else
				return PasswordWrong;
		} else
			return UnknownUser;
	} else
		return UnknownUser;
}

const QString Servatrice::versionString = "Servatrice 0.20100915";
