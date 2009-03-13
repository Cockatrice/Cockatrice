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
#include "testservergame.h"

TestServerGame::TestServerGame(QString _name, QString _description, QString _password, int _maxPlayers, QObject *parent)
	: QObject(parent), name(_name), description(_description), password(_password), maxPlayers(_maxPlayers)
{
	gameStarted = false;
	mutex = new QMutex(QMutex::Recursive);
	rnd = NULL;
}

TestServerGame::~TestServerGame()
{
	if (rnd)
		delete rnd;
	delete mutex;
	qDebug("TestServerGame destructor");
}

bool TestServerGame::getGameStarted()
{
	return gameStarted;
}

int TestServerGame::getPlayerCount()
{
	QMutexLocker locker(mutex);
	return players.size();
}

QStringList TestServerGame::getPlayerNames()
{
	QMutexLocker locker(mutex);
	
	QStringList result;
	QListIterator<TestServerSocket *> i(players);
	while (i.hasNext()) {
		TestServerSocket *tmp = i.next();
		result << QString("%1|%2").arg(tmp->getPlayerId()).arg(tmp->PlayerName);
	}
	return result;
}

TestServerSocket *TestServerGame::getPlayer(int player_id)
{
	QListIterator<TestServerSocket *> i(players);
	while (i.hasNext()) {
		TestServerSocket *tmp = i.next();
		if (tmp->getPlayerId() == player_id)
			return tmp;
	}
	return NULL;
}

void TestServerGame::msg(const QString &s)
{
	QMutexLocker locker(mutex);
	
	QListIterator<TestServerSocket *> i(players);
	while (i.hasNext())
		i.next()->msg(s);
}

void TestServerGame::broadcastEvent(const QString &cmd, TestServerSocket *player)
{
	if (player)
		msg(QString("public|%1|%2|%3").arg(player->getPlayerId()).arg(player->PlayerName).arg(cmd));
	else
		msg(QString("public|||%1").arg(cmd));
}

void TestServerGame::startGameIfReady()
{
	QMutexLocker locker(mutex);
	
	if (players.size() < maxPlayers)
		return;
	for (int i = 0; i < players.size(); i++)
		if (players.at(i)->getStatus() != StatusReadyStart)
			return;
			
	rnd = new TestRandom(this);
	rnd->init();
	
	for (int i = 0; i < players.size(); i++)
		players.at(i)->setupZones();
	
	activePlayer = 0;
	activePhase = 0;
	gameStarted = true;
	broadcastEvent("game_start", NULL);
}

void TestServerGame::addPlayer(TestServerSocket *player)
{
	QMutexLocker locker(mutex);
	
	int max = -1;
	QListIterator<TestServerSocket *> i(players);
	while (i.hasNext()) {
		int tmp = i.next()->getPlayerId();
		if (tmp > max)
			max = tmp;
	}
	player->setPlayerId(max + 1);
	
	player->setGame(this);
	player->msg(QString("private|||player_id|%1|%2").arg(max + 1).arg(player->PlayerName));
	broadcastEvent("join", player);
	
	players << player;
	
	connect(player, SIGNAL(broadcastEvent(const QString &, TestServerSocket *)), this, SLOT(broadcastEvent(const QString &, TestServerSocket *)));
}

void TestServerGame::removePlayer(TestServerSocket *player)
{
	QMutexLocker locker(mutex);
	
	players.removeAt(players.indexOf(player));
	broadcastEvent("leave", player);
	if (!players.size())
		thread()->quit();
}

void TestServerGame::setActivePlayer(int _activePlayer)
{
	activePlayer = _activePlayer;
	broadcastEvent(QString("set_active_player|%1").arg(_activePlayer), NULL);
	setActivePhase(0);
}

void TestServerGame::setActivePhase(int _activePhase)
{
	activePhase = _activePhase;
	broadcastEvent(QString("set_active_phase|%1").arg(_activePhase), NULL);
}
