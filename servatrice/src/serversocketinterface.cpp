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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "serversocketinterface.h"
#include "servatrice.h"
#include "protocol.h"
#include "protocol_items.h"

ServerSocketInterface::ServerSocketInterface(Server *_server, QTcpSocket *_socket, QObject *parent)
	: Server_ProtocolHandler(_server, parent), socket(_socket)
{
	xmlWriter = new QXmlStreamWriter;
	xmlWriter->setDevice(socket);
	xmlWriter->setAutoFormatting(true);
	
	xmlReader = new QXmlStreamReader;
	
	connect(socket, SIGNAL(readyRead()), this, SLOT(readClient()));
	connect(socket, SIGNAL(disconnected()), this, SLOT(deleteLater()));
	connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(catchSocketError(QAbstractSocket::SocketError)));
	
	xmlWriter->writeStartDocument();
	xmlWriter->writeStartElement("cockatrice_communication");
	xmlWriter->writeAttribute("version", QString::number(ProtocolItem::protocolVersion));
	
	sendProtocolItem(new Event_Welcome(Servatrice::versionString));
}

ServerSocketInterface::~ServerSocketInterface()
{
	qDebug("ServerSocketInterface destructor");
	
	delete xmlWriter;
	delete xmlReader;
	delete socket;
/*	clearZones();
	// The socket has to be removed from the server's list before it is removed from the game's list
	// so it will not receive the game update event.
	server->removePlayer(this);
	if (game)
		game->removePlayer(this);
	for (int i = 0; i < chatChannels.size(); ++i)
		chatChannels[i]->removePlayer(this);
*/}

void ServerSocketInterface::readClient()
{
	xmlReader->addData(socket->readAll());
	
	while (canReadLine()) {
		QString line = QString(readLine()).trimmed();
		if (line.isNull())
			break;
			
		qDebug(QString("<<< %1").arg(line).toLatin1());
/*		switch (PlayerStatus) {
			case StatusNormal:
			case StatusReadyStart:
			case StatusPlaying:
				parseCommand(line);
				break;
			case StatusSubmitDeck:
				QString card = line;
				if (card == ".") {
					PlayerStatus = StatusNormal;
					remsg->send(ReturnMessage::ReturnOk);
				} else if (card.startsWith("SB:"))
					SideboardList << card.mid(3);
				else
					DeckList << card;
		}
	}
*/}

void ServerSocketInterface::catchSocketError(QAbstractSocket::SocketError socketError)
{
	qDebug(QString("socket error: %1").arg(socketError).toLatin1());
	
	deleteLater();
}

void ServerSocketInterface::sendProtocolItem(ProtocolItem *item)
{
	item->write(xmlWriter);
	delete item;
}
