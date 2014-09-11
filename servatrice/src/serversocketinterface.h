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
#ifndef SERVERSOCKETINTERFACE_H
#define SERVERSOCKETINTERFACE_H

#include <QTcpSocket>
#include <QHostAddress>
#include <QMutex>
#include "server_protocolhandler.h"

class QTcpSocket;
class Servatrice;
class Servatrice_DatabaseInterface;
class DeckList;
class ServerInfo_DeckStorage_Folder;

class Command_AddToList;
class Command_RemoveFromList;
class Command_DeckList;
class Command_DeckNewDir;
class Command_DeckDelDir;
class Command_DeckDel;
class Command_DeckDownload;
class Command_DeckUpload;
class Command_ReplayList;
class Command_ReplayDownload;
class Command_ReplayModifyMatch;
class Command_ReplayDeleteMatch;

class Command_BanFromServer;
class Command_UpdateServerMessage;
class Command_ShutdownServer;

class ServerSocketInterface : public Server_ProtocolHandler
{
	Q_OBJECT
private slots:
	void readClient();
	void catchSocketError(QAbstractSocket::SocketError socketError);
	void flushOutputQueue();
signals:
	void outputQueueChanged();
protected:
	void logDebugMessage(const QString &message);
private:
	QMutex outputQueueMutex;
	Servatrice *servatrice;
	Servatrice_DatabaseInterface *sqlInterface;
	QTcpSocket *socket;
	
	QByteArray inputBuffer;
	QList<ServerMessage> outputQueue;
	bool messageInProgress;
	bool handshakeStarted;
	int messageLength;
	
	Response::ResponseCode cmdAddToList(const Command_AddToList &cmd, ResponseContainer &rc);
	Response::ResponseCode cmdRemoveFromList(const Command_RemoveFromList &cmd, ResponseContainer &rc);
	int getDeckPathId(int basePathId, QStringList path);
	int getDeckPathId(const QString &path);
	bool deckListHelper(int folderId, ServerInfo_DeckStorage_Folder *folder);
	Response::ResponseCode cmdDeckList(const Command_DeckList &cmd, ResponseContainer &rc);
	Response::ResponseCode cmdDeckNewDir(const Command_DeckNewDir &cmd, ResponseContainer &rc);
	void deckDelDirHelper(int basePathId);
	Response::ResponseCode cmdDeckDelDir(const Command_DeckDelDir &cmd, ResponseContainer &rc);
	Response::ResponseCode cmdDeckDel(const Command_DeckDel &cmd, ResponseContainer &rc);
	Response::ResponseCode cmdDeckUpload(const Command_DeckUpload &cmd, ResponseContainer &rc);
	DeckList *getDeckFromDatabase(int deckId);
	Response::ResponseCode cmdDeckDownload(const Command_DeckDownload &cmd, ResponseContainer &rc);
	Response::ResponseCode cmdReplayList(const Command_ReplayList &cmd, ResponseContainer &rc);
	Response::ResponseCode cmdReplayDownload(const Command_ReplayDownload &cmd, ResponseContainer &rc);
	Response::ResponseCode cmdReplayModifyMatch(const Command_ReplayModifyMatch &cmd, ResponseContainer &rc);
	Response::ResponseCode cmdReplayDeleteMatch(const Command_ReplayDeleteMatch &cmd, ResponseContainer &rc);
	Response::ResponseCode cmdBanFromServer(const Command_BanFromServer &cmd, ResponseContainer &rc);
	Response::ResponseCode cmdShutdownServer(const Command_ShutdownServer &cmd, ResponseContainer &rc);
	Response::ResponseCode cmdUpdateServerMessage(const Command_UpdateServerMessage &cmd, ResponseContainer &rc);
	
	Response::ResponseCode processExtendedSessionCommand(int cmdType, const SessionCommand &cmd, ResponseContainer &rc);
	Response::ResponseCode processExtendedModeratorCommand(int cmdType, const ModeratorCommand &cmd, ResponseContainer &rc);
	Response::ResponseCode processExtendedAdminCommand(int cmdType, const AdminCommand &cmd, ResponseContainer &rc);
public:
	ServerSocketInterface(Servatrice *_server, Servatrice_DatabaseInterface *_databaseInterface, QObject *parent = 0);
	~ServerSocketInterface();
	void initSessionDeprecated();
	bool initSession();
	QHostAddress getPeerAddress() const { return socket->peerAddress(); }
	QString getAddress() const { return socket->peerAddress().toString(); }

	void transmitProtocolItem(const ServerMessage &item);
public slots:
	void initConnection(int socketDescriptor);
};

#endif
