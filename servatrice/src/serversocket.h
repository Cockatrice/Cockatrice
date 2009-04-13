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
#include <QList>
#include <QVariant>
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
	typedef ReturnMessage::ReturnCode (ServerSocket::*CommandHandler)(const QList<QVariant> &);
	struct CommandProperties {
		QString name;
		bool needsLogin;
		bool needsGame;
		bool needsStartedGame;
		QList<QVariant::Type> paramTypes;
		CommandHandler handler;
	};
	static const int numberCommands = 24;
	static const CommandProperties commandList[numberCommands];

	ReturnMessage::ReturnCode cmdLogin(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdListGames(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdCreateGame(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdJoinGame(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdLeaveGame(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdListPlayers(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdSay(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdSubmitDeck(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdReadyStart(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdShuffle(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdDrawCards(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdMoveCard(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdCreateToken(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdSetCardAttr(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdIncCounter(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdAddCounter(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdSetCounter(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdDelCounter(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdListCounters(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdListZones(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdDumpZone(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdRollDice(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdSetActivePlayer(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdSetActivePhase(const QList<QVariant> &params);

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
