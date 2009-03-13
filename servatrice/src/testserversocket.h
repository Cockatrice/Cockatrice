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
#ifndef TESTSERVERSOCKET_H
#define TESTSERVERSOCKET_H

#include <QTcpSocket>
#include "testserver.h"
#include "testservergame.h"
#include "playerzone.h"
#include "returnmessage.h"
#include "counter.h"

class TestServer;
class TestServerGame;
class PlayerZone;

enum PlayerStatusEnum { StatusNormal, StatusSubmitDeck, StatusReadyStart, StatusPlaying };

class TestServerSocket : public QTcpSocket
{
	Q_OBJECT
private slots:
	void readClient();
	void catchSocketError(QAbstractSocket::SocketError socketError);
signals:
	void createGame(const QString name, const QString description, const QString password, const int maxPlayers, TestServerSocket *creator);
	void joinGame(const QString name, TestServerSocket *player);
	void commandReceived(QString cmd, TestServerSocket *player);
	void broadcastEvent(const QString &event, TestServerSocket *player);
	void startGameIfReady();
private:
	TestServer *server;
	TestServerGame *game;
	QList<QString> DeckList;
	QList<QString> SideboardList;
	QList<PlayerZone *> zones;
	QList<Counter *> counters;
	int PlayerId;
	int nextCardId;
	PlayerZone *getZone(const QString &name);
	Counter *getCounter(const QString &name);
	void setName(const QString &name);
	void clearZones();
	void leaveGame();
	bool parseCommand(QString line);
	PlayerStatusEnum PlayerStatus;
	ReturnMessage *remsg;
public:
	QString PlayerName;
	TestServerSocket(TestServer *_server, QObject *parent = 0);
	~TestServerSocket();
	void msg(const QString &s);
	void setGame(TestServerGame *g);
	PlayerStatusEnum getStatus();
	void setStatus(PlayerStatusEnum status);
	void initConnection();
	int getPlayerId() { return PlayerId; }
	void setPlayerId(int _id) { PlayerId = _id; }
	QStringList listCounters();
	QStringList listZones();
	void setupZones();
};

#endif
