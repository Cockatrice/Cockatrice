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
#if QT_VERSION > 0x050300
  #include <QWebSocket>
#endif
#include <QHostAddress>
#include <QMutex>
#include "server_protocolhandler.h"

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
class Command_ReloadConfig;

class Command_AccountEdit;
class Command_AccountImage;
class Command_AccountPassword;

class AbstractServerSocketInterface : public Server_ProtocolHandler
{
	Q_OBJECT
protected slots:
	void catchSocketError(QAbstractSocket::SocketError socketError);
	virtual void flushOutputQueue() = 0;
signals:
	void outputQueueChanged();
protected:
	void logDebugMessage(const QString &message);
	bool tooManyRegistrationAttempts(const QString &ipAddress);

	virtual void writeToSocket(QByteArray & data) = 0;
	virtual void flushSocket() = 0;

	Servatrice *servatrice;
	QList<ServerMessage> outputQueue;
	QMutex outputQueueMutex;
private:
	Servatrice_DatabaseInterface *sqlInterface;
		
	Response::ResponseCode cmdAddToList(const Command_AddToList &cmd, ResponseContainer &rc);
	Response::ResponseCode cmdRemoveFromList(const Command_RemoveFromList &cmd, ResponseContainer &rc);
	int getDeckPathId(int basePathId, QStringList path);
	int getDeckPathId(const QString &path);
	bool deckListHelper(int folderId, ServerInfo_DeckStorage_Folder *folder);
	Response::ResponseCode cmdDeckList(const Command_DeckList &cmd, ResponseContainer &rc);
	Response::ResponseCode cmdDeckNewDir(const Command_DeckNewDir &cmd, ResponseContainer &rc);
	void deckDelDirHelper(int basePathId);
	void sendServerMessage(const QString userName, const QString message);
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
	Response::ResponseCode cmdWarnUser(const Command_WarnUser &cmd, ResponseContainer &rc);
	Response::ResponseCode cmdGetLogHistory(const Command_ViewLogHistory &cmd, ResponseContainer &rc);
	Response::ResponseCode cmdGetBanHistory(const Command_GetBanHistory &cmd, ResponseContainer &rc);
	Response::ResponseCode cmdGetWarnList(const Command_GetWarnList &cmd, ResponseContainer &rc);
	Response::ResponseCode cmdGetWarnHistory(const Command_GetWarnHistory &cmd, ResponseContainer &rc);
	Response::ResponseCode cmdShutdownServer(const Command_ShutdownServer &cmd, ResponseContainer &rc);
	Response::ResponseCode cmdUpdateServerMessage(const Command_UpdateServerMessage &cmd, ResponseContainer &rc);
    Response::ResponseCode cmdRegisterAccount(const Command_Register &cmd, ResponseContainer &rc);
    Response::ResponseCode cmdActivateAccount(const Command_Activate &cmd, ResponseContainer & /* rc */);
    Response::ResponseCode cmdReloadConfig(const Command_ReloadConfig &/* cmd */, ResponseContainer & /*rc*/);
	Response::ResponseCode cmdAdjustMod(const Command_AdjustMod &cmd, ResponseContainer & /*rc*/);
	
	Response::ResponseCode processExtendedSessionCommand(int cmdType, const SessionCommand &cmd, ResponseContainer &rc);
	Response::ResponseCode processExtendedModeratorCommand(int cmdType, const ModeratorCommand &cmd, ResponseContainer &rc);
	Response::ResponseCode processExtendedAdminCommand(int cmdType, const AdminCommand &cmd, ResponseContainer &rc);

	Response::ResponseCode cmdAccountEdit(const Command_AccountEdit &cmd, ResponseContainer &rc);
	Response::ResponseCode cmdAccountImage(const Command_AccountImage &cmd, ResponseContainer &rc);
	Response::ResponseCode cmdAccountPassword(const Command_AccountPassword &cmd, ResponseContainer &rc);
public:
	AbstractServerSocketInterface(Servatrice *_server, Servatrice_DatabaseInterface *_databaseInterface, QObject *parent = 0);
	~AbstractServerSocketInterface() { };
	bool initSession();

	virtual QHostAddress getPeerAddress() const = 0;
	virtual QString getAddress() const = 0;

	void transmitProtocolItem(const ServerMessage &item);
};

class TcpServerSocketInterface : public AbstractServerSocketInterface
{
	Q_OBJECT
public:
	TcpServerSocketInterface(Servatrice *_server, Servatrice_DatabaseInterface *_databaseInterface, QObject *parent = 0);
	~TcpServerSocketInterface();

	QHostAddress getPeerAddress() const { return socket->peerAddress(); }
	QString getAddress() const { return socket->peerAddress().toString(); }
	QString getConnectionType() const { return "tcp"; };
private:
	QTcpSocket *socket;
	QByteArray inputBuffer;
	bool messageInProgress;
	bool handshakeStarted;
	int messageLength;
protected:
	void writeToSocket(QByteArray & data) { socket->write(data); };
	void flushSocket() { socket->flush(); };
	void initSessionDeprecated();
	bool initTcpSession();
protected slots:
	void readClient();
	void flushOutputQueue();
public slots:
	void initConnection(int socketDescriptor);
};

#if QT_VERSION > 0x050300
class WebsocketServerSocketInterface : public AbstractServerSocketInterface
{
	Q_OBJECT
public:
	WebsocketServerSocketInterface(Servatrice *_server, Servatrice_DatabaseInterface *_databaseInterface, QObject *parent = 0);
	~WebsocketServerSocketInterface();

	QHostAddress getPeerAddress() const { return socket->peerAddress(); }
	QString getAddress() const { return socket->peerAddress().toString(); }
	QString getConnectionType() const { return "websocket"; };
private:
	QWebSocket *socket;
protected:
	void writeToSocket(QByteArray & data) { socket->sendBinaryMessage(data); };
	void flushSocket() { socket->flush(); };
	bool initWebsocketSession();
protected slots:
	void binaryMessageReceived(const QByteArray & message);
	void flushOutputQueue();
public slots:
	void initConnection(void * _socket);
};
#endif

#endif
