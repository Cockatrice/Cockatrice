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
#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

#include <QTcpSocket>
#include "server.h"
#include "returnmessage.h"

class Server;
class ServerGame;
class PlayerZone;
class Counter;

enum PlayerStatusEnum { StatusNormal, StatusSubmitDeck, StatusReadyStart, StatusPlaying };

class ServerSocket : public QTcpSocket
{
	Q_OBJECT
private slots:
	void readClient();
	void catchSocketError(QAbstractSocket::SocketError socketError);
signals:
	void createGame(const QString name, const QString description, const QString password, const int maxPlayers, ServerSocket *creator);
	void joinGame(const QString name, ServerSocket *player);
	void commandReceived(QString cmd, ServerSocket *player);
	void broadcastEvent(const QString &event, ServerSocket *player);
	void startGameIfReady();
private:
	typedef ReturnMessage::ReturnCode (ServerSocket::*CommandHandler)(const QStringList &);
	struct CommandProperties {
		QString name;
		int numberParams;
		bool needsLogin;
		bool needsGame;
		bool needsStartedGame;
		CommandHandler handler;
	};
	static const int numberCommands = 23;
	static const CommandProperties commandList[numberCommands];

	ReturnMessage::ReturnCode cmdLogin(const QStringList &params);
	ReturnMessage::ReturnCode cmdListGames(const QStringList &params);
	ReturnMessage::ReturnCode cmdCreateGame(const QStringList &params);
	ReturnMessage::ReturnCode cmdJoinGame(const QStringList &params);
	ReturnMessage::ReturnCode cmdLeaveGame(const QStringList &params);
	ReturnMessage::ReturnCode cmdListPlayers(const QStringList &params);
	ReturnMessage::ReturnCode cmdSay(const QStringList &params);
	ReturnMessage::ReturnCode cmdSubmitDeck(const QStringList &params);
	ReturnMessage::ReturnCode cmdReadyStart(const QStringList &params);
	ReturnMessage::ReturnCode cmdShuffle(const QStringList &params);
	ReturnMessage::ReturnCode cmdDrawCards(const QStringList &params);
	ReturnMessage::ReturnCode cmdMoveCard(const QStringList &params);
	ReturnMessage::ReturnCode cmdCreateToken(const QStringList &params);
	ReturnMessage::ReturnCode cmdSetCardAttr(const QStringList &params);
	ReturnMessage::ReturnCode cmdIncCounter(const QStringList &params);
	ReturnMessage::ReturnCode cmdSetCounter(const QStringList &params);
	ReturnMessage::ReturnCode cmdDelCounter(const QStringList &params);
	ReturnMessage::ReturnCode cmdListCounters(const QStringList &params);
	ReturnMessage::ReturnCode cmdListZones(const QStringList &params);
	ReturnMessage::ReturnCode cmdDumpZone(const QStringList &params);
	ReturnMessage::ReturnCode cmdRollDice(const QStringList &params);
	ReturnMessage::ReturnCode cmdSetActivePlayer(const QStringList &params);
	ReturnMessage::ReturnCode cmdSetActivePhase(const QStringList &params);

	Server *server;
	ServerGame *game;
	QList<QString> DeckList;
	QList<QString> SideboardList;
	QList<PlayerZone *> zones;
	QList<Counter *> counters;
	int PlayerId;
	int nextCardId;
	int newCardId();
	PlayerZone *getZone(const QString &name);
	Counter *getCounter(const QString &name);
	void setName(const QString &name);
	void clearZones();
	void leaveGame();
	bool parseCommand(QString line);
	PlayerStatusEnum PlayerStatus;
	ReturnMessage *remsg;
	AuthenticationResult authState;
public:
	QString PlayerName;
	ServerSocket(Server *_server, QObject *parent = 0);
	~ServerSocket();
	void msg(const QString &s);
	void setGame(ServerGame *g);
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
