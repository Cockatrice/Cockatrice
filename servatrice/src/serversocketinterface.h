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
#include "server_protocolhandler.h"

class QTcpSocket;
class Servatrice;
class QXmlStreamReader;
class QXmlStreamWriter;

class ServerSocketInterface : public Server_ProtocolHandler
{
	Q_OBJECT
private slots:
	void readClient();
	void catchSocketError(QAbstractSocket::SocketError socketError);
private:
	Servatrice *servatrice;
	QTcpSocket *socket;
	QXmlStreamWriter *xmlWriter;
	QXmlStreamReader *xmlReader;
	ProtocolItem *currentItem;

	int getDeckPathId(int basePathId, QStringList path);
	int getDeckPathId(const QString &path);
	bool deckListHelper(DeckList_Directory *folder);
	ResponseCode cmdDeckList(Command_DeckList *cmd);
	ResponseCode cmdDeckNewDir(Command_DeckNewDir *cmd);
	void deckDelDirHelper(int basePathId);
	ResponseCode cmdDeckDelDir(Command_DeckDelDir *cmd);
	ResponseCode cmdDeckDel(Command_DeckDel *cmd);
	ResponseCode cmdDeckUpload(Command_DeckUpload *cmd);
	ResponseCode cmdDeckDownload(Command_DeckDownload *cmd);
	
	void itemFinishedReading();
public:
	ServerSocketInterface(Servatrice *_server, QTcpSocket *_socket, QObject *parent = 0);
	~ServerSocketInterface();

	void sendProtocolItem(ProtocolItem *item, bool deleteItem = true);
};

#endif
