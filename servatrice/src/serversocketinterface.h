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
class QXmlStreamReader;
class QXmlStreamWriter;
class DeckList;
class TopLevelProtocolItem;
class QByteArray;

class ServerSocketInterface : public Server_ProtocolHandler
{
	Q_OBJECT
private slots:
	void readClient();
	void catchSocketError(QAbstractSocket::SocketError socketError);
//	void processProtocolItem(ProtocolItem *item);
	void flushXmlBuffer();
signals:
	void xmlBufferChanged();
private:
	QMutex xmlBufferMutex;
	Servatrice *servatrice;
	QTcpSocket *socket;
	QXmlStreamWriter *xmlWriter;
	QXmlStreamReader *xmlReader;
	QString xmlBuffer;
	TopLevelProtocolItem *topLevelItem;
	bool compressionSupport;
	
	QByteArray inputBuffer;
	bool messageInProgress;
	int messageLength;
	
	int getUserIdInDB(const QString &name) const;

	ResponseCode cmdAddToList(const Command_AddToList &cmd, BlaContainer *bla);
	ResponseCode cmdRemoveFromList(const Command_RemoveFromList &cmd, BlaContainer *bla);
	int getDeckPathId(int basePathId, QStringList path);
	int getDeckPathId(const QString &path);
	bool deckListHelper(DeckList_Directory *folder);
	ResponseCode cmdDeckList(const Command_DeckList &cmd, BlaContainer *bla);
	ResponseCode cmdDeckNewDir(const Command_DeckNewDir &cmd, BlaContainer *bla);
	void deckDelDirHelper(int basePathId);
	ResponseCode cmdDeckDelDir(const Command_DeckDelDir &cmd, BlaContainer *bla);
	ResponseCode cmdDeckDel(const Command_DeckDel &cmd, BlaContainer *bla);
	ResponseCode cmdDeckUpload(const Command_DeckUpload &cmd, BlaContainer *bla);
	DeckList *getDeckFromDatabase(int deckId);
	ResponseCode cmdDeckDownload(const Command_DeckDownload &cmd, BlaContainer *bla);
	ResponseCode cmdBanFromServer(const Command_BanFromServer &cmd, BlaContainer *bla);
	ResponseCode cmdShutdownServer(const Command_ShutdownServer &cmd, BlaContainer *bla);
	ResponseCode cmdUpdateServerMessage(const Command_UpdateServerMessage &cmd, BlaContainer *bla);
protected:
	bool getCompressionSupport() const { return compressionSupport; }
public:
	ServerSocketInterface(Servatrice *_server, QTcpSocket *_socket, QObject *parent = 0);
	~ServerSocketInterface();
	QHostAddress getPeerAddress() const { return socket->peerAddress(); }
	QString getAddress() const { return socket->peerAddress().toString(); }

	void sendProtocolItem(ProtocolItem *item, bool deleteItem = true);
};

#endif
