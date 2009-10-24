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
#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QStringList>

class ServerGame;
class ServerSocket;
class QSqlDatabase;
class QSettings;
class ChatChannel;

enum AuthenticationResult { PasswordWrong = 0, PasswordRight = 1, UnknownUser = 2 };

class Server : public QTcpServer
{
	Q_OBJECT
private slots:
	void gameClosing();
	void broadcastChannelUpdate();
public:
	Server(QObject *parent = 0);
	~Server();
	QSettings *settings;
	bool openDatabase();
	AuthenticationResult checkUserPassword(const QString &user, const QString &password);
	QList<ServerGame *> getGames() const { return games.values(); }
	ServerGame *getGame(int gameId) const;
	const QMap<QString, ChatChannel *> &getChatChannels() { return chatChannels; }
	void broadcastGameListUpdate(ServerGame *game);
	void removePlayer(ServerSocket *player);
	const QStringList &getLoginMessage() const { return loginMessage; }
	ServerGame *createGame(const QString &description, const QString &password, int maxPlayers, bool spectatorsAllowed, const QString &playerName);
private:
	void incomingConnection(int SocketId);
	QMap<int, ServerGame *> games;
	QList<ServerSocket *> players;
	QMap<QString, ChatChannel *> chatChannels;
	int nextGameId;
	QStringList loginMessage;
};

#endif
