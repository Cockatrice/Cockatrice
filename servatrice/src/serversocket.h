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
	void createGame(const QString description, const QString password, int maxPlayers, bool spectatorsAllowed, ServerSocket *creator);
	void commandReceived(QString cmd, ServerSocket *player);
	void broadcastEvent(const QString &event, ServerSocket *player);
	void startGameIfReady();
private:
	typedef ReturnMessage::ReturnCode (ServerSocket::*CommandHandler)(const QList<QVariant> &);
	class CommandProperties {
	private:
		bool needsLogin;
		bool needsGame;
		bool needsStartedGame;
		bool allowedToSpectator;
		QList<QVariant::Type> paramTypes;
		CommandHandler handler;
	public:
		CommandProperties(bool _needsLogin = false, bool _needsGame = false, bool _needsStartedGame = false, bool _allowedToSpectator = false, const QList<QVariant::Type> &_paramTypes = QList<QVariant::Type>(), CommandHandler _handler = 0)
			: needsLogin(_needsLogin), needsGame(_needsGame), needsStartedGame(_needsStartedGame), allowedToSpectator(_allowedToSpectator), paramTypes(_paramTypes), handler(_handler) { }
		bool getNeedsLogin() const { return needsLogin; }
		bool getNeedsGame() const { return needsGame; }
		bool getNeedsStartedGame() const { return needsStartedGame; }
		bool getAllowedToSpectator() const { return allowedToSpectator; }
		const QList<QVariant::Type> &getParamTypes() const { return paramTypes; }
		CommandHandler getHandler() const { return handler; }
	};
	static QHash<QString, CommandProperties> commandHash;

	QStringList listPlayersHelper();
	QStringList listZonesHelper(ServerSocket *player);
	QStringList dumpZoneHelper(ServerSocket *player, PlayerZone *zone, int numberCards);
	QStringList listCountersHelper(ServerSocket *player);
	
	ReturnMessage::ReturnCode cmdPing(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdLogin(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdChatListChannels(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdChatJoinChannel(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdChatLeaveChannel(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdChatSay(const QList<QVariant> &params);
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
	ReturnMessage::ReturnCode cmdRevealCard(const QList<QVariant> &params);
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
	ReturnMessage::ReturnCode cmdStopDumpZone(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdRollDie(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdNextTurn(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdSetActivePhase(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdDumpAll(const QList<QVariant> &params);

	Server *server;
	ServerGame *game;
	QList<ChatChannel *> chatChannels;
	QList<QString> DeckList;
	QList<QString> SideboardList;
	QList<PlayerZone *> zones;
	QList<Counter *> counters;
	int playerId;
	QString playerName;
	bool spectator;
	int nextCardId;
	int newCardId();
	PlayerZone *getZone(const QString &name) const;
	Counter *getCounter(const QString &name) const;
	void clearZones();
	bool parseCommand(QString line);
	PlayerStatusEnum PlayerStatus;
	ReturnMessage *remsg;
	AuthenticationResult authState;
	bool acceptsGameListChanges;
	bool acceptsChatChannelListChanges;
public:
	ServerSocket(Server *_server, QObject *parent = 0);
	~ServerSocket();
	void msg(const QString &s);
	void privateEvent(const QString &line);
	void publicEvent(const QString &line, ServerSocket *player = 0);
	void setGame(ServerGame *g) { game = g; }
	void leaveGame();
	PlayerStatusEnum getStatus() { return PlayerStatus; }
	void setStatus(PlayerStatusEnum _status) { PlayerStatus = _status; }
	void initConnection();
	int getPlayerId() const { return playerId; }
	void setPlayerId(int _id) { playerId = _id; }
	bool getSpectator() const { return spectator; }
	QString getPlayerName() const { return playerName; }
	bool getAcceptsGameListChanges() const { return acceptsGameListChanges; }
	bool getAcceptsChatChannelListChanges() const { return acceptsChatChannelListChanges; }
	const QList<PlayerZone *> &getZones() const { return zones; }
	const QList<Counter *> &getCounters() const { return counters; }
	void setupZones();
};

#endif
