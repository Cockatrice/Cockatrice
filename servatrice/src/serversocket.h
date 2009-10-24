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
class Player;
class PlayerZone;
class Counter;
class Arrow;

class ServerSocket : public QTcpSocket
{
	Q_OBJECT
private slots:
	void readClient();
	void catchSocketError(QAbstractSocket::SocketError socketError);
signals:
	void commandReceived(QString cmd, ServerSocket *player);
	void startGameIfReady();
private:
	typedef ReturnMessage::ReturnCode (ServerSocket::*GameCommandHandler)(ServerGame *game, Player *player, const QList<QVariant> &);
	typedef ReturnMessage::ReturnCode (ServerSocket::*ChatCommandHandler)(ChatChannel *channel, const QList<QVariant> &);
	typedef ReturnMessage::ReturnCode (ServerSocket::*GenericCommandHandler)(const QList<QVariant> &);
	
	class CommandProperties {
	public:
		enum CommandType { ChatCommand, GameCommand, GenericCommand };
	private:
		CommandType type;
		bool needsLogin;
		QList<QVariant::Type> paramTypes;
	protected:
		QList<QVariant> getParamList(const QStringList &params) const;
	public:
		CommandProperties(CommandType _type, bool _needsLogin, const QList<QVariant::Type> &_paramTypes)
			: type(_type), needsLogin(_needsLogin), paramTypes(_paramTypes) { }
		bool getNeedsLogin() const { return needsLogin; }
		CommandType getType() const { return type; }
		const QList<QVariant::Type> &getParamTypes() const { return paramTypes; }
		virtual ReturnMessage::ReturnCode exec(ServerSocket *s, QStringList &params) = 0;
	};
	class ChatCommandProperties : public CommandProperties {
	private:
		ChatCommandHandler handler;
	public:
		ChatCommandProperties(const QList<QVariant::Type> &_paramTypes, ChatCommandHandler _handler)
			: CommandProperties(ChatCommand, true, _paramTypes), handler(_handler) { }
		ReturnMessage::ReturnCode exec(ServerSocket *s, QStringList &params);
	};
	class GameCommandProperties : public CommandProperties {
	private:
		bool needsStartedGame;
		bool allowedToSpectator;
		GameCommandHandler handler;
	public:
		GameCommandProperties(bool _needsStartedGame, bool _allowedToSpectator, const QList<QVariant::Type> &_paramTypes, GameCommandHandler _handler)
			: CommandProperties(GameCommand, true, _paramTypes), needsStartedGame(_needsStartedGame), allowedToSpectator(_allowedToSpectator), handler(_handler) { }
		bool getNeedsStartedGame() const { return needsStartedGame; }
		bool getAllowedToSpectator() const { return allowedToSpectator; }
		ReturnMessage::ReturnCode exec(ServerSocket *s, QStringList &params);
	};
	class GenericCommandProperties : public CommandProperties {
	private:
		GenericCommandHandler handler;
	public:
		GenericCommandProperties(bool _needsLogin, const QList<QVariant::Type> &_paramTypes, GenericCommandHandler _handler)
			: CommandProperties(GenericCommand, _needsLogin, _paramTypes), handler(_handler) { }
		ReturnMessage::ReturnCode exec(ServerSocket *s, QStringList &params);
	};
	static QHash<QString, CommandProperties *> commandHash;

	QStringList listPlayersHelper(ServerGame *game, Player *player);
	QStringList listZonesHelper(Player *player);
	QStringList dumpZoneHelper(Player *player, PlayerZone *zone, int numberCards);
	QStringList listCountersHelper(Player *player);
	QStringList listArrowsHelper(Player *player);
	
	ReturnMessage::ReturnCode cmdPing(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdLogin(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdChatListChannels(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdChatJoinChannel(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdListGames(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdCreateGame(const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdJoinGame(const QList<QVariant> &params);
	
	ReturnMessage::ReturnCode cmdChatLeaveChannel(ChatChannel *channel, const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdChatSay(ChatChannel *channel, const QList<QVariant> &params);
	
	ReturnMessage::ReturnCode cmdLeaveGame(ServerGame *game, Player *player, const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdListPlayers(ServerGame *game, Player *player, const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdSay(ServerGame *game, Player *player, const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdSubmitDeck(ServerGame *game, Player *player, const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdReadyStart(ServerGame *game, Player *player, const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdShuffle(ServerGame *game, Player *player, const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdDrawCards(ServerGame *game, Player *player, const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdRevealCard(ServerGame *game, Player *player, const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdMoveCard(ServerGame *game, Player *player, const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdCreateToken(ServerGame *game, Player *player, const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdCreateArrow(ServerGame *game, Player *player, const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdDeleteArrow(ServerGame *game, Player *player, const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdSetCardAttr(ServerGame *game, Player *player, const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdIncCounter(ServerGame *game, Player *player, const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdAddCounter(ServerGame *game, Player *player, const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdSetCounter(ServerGame *game, Player *player, const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdDelCounter(ServerGame *game, Player *player, const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdListCounters(ServerGame *game, Player *player, const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdListZones(ServerGame *game, Player *player, const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdDumpZone(ServerGame *game, Player *player, const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdStopDumpZone(ServerGame *game, Player *player, const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdRollDie(ServerGame *game, Player *player, const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdNextTurn(ServerGame *game, Player *player, const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdSetActivePhase(ServerGame *game, Player *player, const QList<QVariant> &params);
	ReturnMessage::ReturnCode cmdDumpAll(ServerGame *game, Player *player, const QList<QVariant> &params);

	Server *server;
	QMap<int, QPair<ServerGame *, Player *> > games;
	QMap<QString, ChatChannel *> chatChannels;
	QString playerName;
	
	Server *getServer() const { return server; }
	QPair<ServerGame *, Player *> getGame(int gameId) const;
	
	bool parseCommand(const QString &line);
	ReturnMessage *remsg;
	AuthenticationResult authState;
	bool acceptsGameListChanges;
	bool acceptsChatChannelListChanges;
public:
	ServerSocket(Server *_server, QObject *parent = 0);
	~ServerSocket();
	void msg(const QString &s);
	void initConnection();
	bool getAcceptsGameListChanges() const { return acceptsGameListChanges; }
	bool getAcceptsChatChannelListChanges() const { return acceptsChatChannelListChanges; }
	const QString &getPlayerName() const { return playerName; }
};

#endif
