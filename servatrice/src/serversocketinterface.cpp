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
#include <QtSql>
#include "serversocketinterface.h"
#include "servatrice.h"
#include "protocol.h"
#include "protocol_items.h"

ServerSocketInterface::ServerSocketInterface(Servatrice *_server, QTcpSocket *_socket, QObject *parent)
	: Server_ProtocolHandler(_server, parent), servatrice(_server), socket(_socket), currentItem(0)
{
	xmlWriter = new QXmlStreamWriter;
	xmlWriter->setDevice(socket);
	xmlWriter->setAutoFormatting(true);
	
	xmlReader = new QXmlStreamReader;
	
	connect(socket, SIGNAL(readyRead()), this, SLOT(readClient()));
	connect(socket, SIGNAL(disconnected()), this, SLOT(deleteLater()));
	connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(catchSocketError(QAbstractSocket::SocketError)));
	
	xmlWriter->writeStartDocument();
	xmlWriter->writeStartElement("cockatrice_server_stream");
	xmlWriter->writeAttribute("version", QString::number(ProtocolItem::protocolVersion));
	
	sendProtocolItem(new Event_ServerMessage(Servatrice::versionString));
}

ServerSocketInterface::~ServerSocketInterface()
{
	qDebug("ServerSocketInterface destructor");
	
	delete xmlWriter;
	delete xmlReader;
	delete socket;
}

void ServerSocketInterface::itemFinishedReading()
{
	Command *command = qobject_cast<Command *>(currentItem);
	if (qobject_cast<InvalidCommand *>(command))
		sendProtocolItem(new ProtocolResponse(command->getCmdId(), RespInvalidCommand));
	else
		processCommand(command);
	currentItem = 0;
}

void ServerSocketInterface::readClient()
{
	xmlReader->addData(socket->readAll());
	
	if (currentItem) {
		if (!currentItem->read(xmlReader))
			return;
		itemFinishedReading();
	}
	while (!xmlReader->atEnd()) {
		xmlReader->readNext();
		if (xmlReader->isStartElement()) {
			QString itemType = xmlReader->name().toString();
			if (itemType == "cockatrice_client_stream")
				continue;
			QString itemName = xmlReader->attributes().value("name").toString();
			qDebug() << "parseXml: startElement: " << "type =" << itemType << ", name =" << itemName;
			currentItem = ProtocolItem::getNewItem(itemType + itemName);
			if (!currentItem)
				currentItem = new InvalidCommand;
			if (!currentItem->read(xmlReader))
				return;
			itemFinishedReading();
		}
	}
}

void ServerSocketInterface::catchSocketError(QAbstractSocket::SocketError socketError)
{
	qDebug(QString("socket error: %1").arg(socketError).toLatin1());
	
	deleteLater();
}

void ServerSocketInterface::sendProtocolItem(ProtocolItem *item, bool deleteItem)
{
	item->write(xmlWriter);
	if (deleteItem)
		delete item;
}

int ServerSocketInterface::getDeckPathId(int basePathId, QStringList path)
{
	if (path.isEmpty())
		return 0;
	if (path[0].isEmpty())
		return 0;
	
	servatrice->checkSql();
	
	QSqlQuery query;
	query.prepare("select id from decklist_folders where id_parent = :id_parent and name = :name and user = :user");
	query.bindValue(":id_parent", basePathId);
	query.bindValue(":name", path.takeFirst());
	query.bindValue(":user", playerName);
	if (!servatrice->execSqlQuery(query))
		return -1;
	if (!query.next())
		return -1;
	int id = query.value(0).toInt();
	if (path.isEmpty())
		return id;
	else
		return getDeckPathId(id, path);
}

int ServerSocketInterface::getDeckPathId(const QString &path)
{
	return getDeckPathId(0, path.split("/"));
}

void ServerSocketInterface::deckListHelper(Response_DeckList::Directory *folder)
{
	QSqlQuery query;
	query.prepare("select id, name from decklist_folders where id_parent = :id_parent and user = :user");
	query.bindValue(":id_parent", folder->getId());
	query.bindValue(":user", playerName);
	servatrice->execSqlQuery(query);
	
	while (query.next()) {
		Response_DeckList::Directory *newFolder = new Response_DeckList::Directory(query.value(1).toString(), query.value(0).toInt());
		folder->append(newFolder);
		deckListHelper(newFolder);
	}
	
	query.prepare("select id, name from decklist_files where id_folder = :id_folder");
	query.bindValue(":id_folder", folder->getId());
	servatrice->execSqlQuery(query);
	
	while (query.next()) {
		Response_DeckList::File *newFile = new Response_DeckList::File(query.value(1).toString(), query.value(0).toInt());
		folder->append(newFile);
	}
}

// CHECK AUTHENTICATION!
// Also check for every function that data belonging to other users cannot be accessed.

ResponseCode ServerSocketInterface::cmdDeckList(Command_DeckList *cmd)
{
	Response_DeckList::Directory *root = new Response_DeckList::Directory(QString());
	
	servatrice->checkSql();
	QSqlQuery query;
	deckListHelper(root);
	
	sendProtocolItem(new Response_DeckList(cmd->getCmdId(), RespOk, root));
	
	return RespNothing;
}

ResponseCode ServerSocketInterface::cmdDeckNewDir(Command_DeckNewDir *cmd)
{
	int folderId = getDeckPathId(cmd->getPath());
	if (folderId == -1)
		return RespNameNotFound;
	
	QSqlQuery query;
	query.prepare("insert into decklist_folders (id_parent, user, name) values(:id_parent, :user, :name)");
	query.bindValue(":id_parent", folderId);
	query.bindValue(":user", playerName);
	query.bindValue(":name", cmd->getDirName());
	if (!servatrice->execSqlQuery(query))
		return RespContextError;
	return RespOk;
}

void ServerSocketInterface::deckDelDirHelper(int basePathId)
{
	QSqlQuery query;
	
	query.prepare("select id from decklist_folders where id_parent = :id_parent");
	query.bindValue(":id_parent", basePathId);
	servatrice->execSqlQuery(query);
	while (query.next())
		deckDelDirHelper(query.value(0).toInt());
	
	query.prepare("delete from decklist_files where id_folder = :id_folder");
	query.bindValue(":id_folder", basePathId);
	servatrice->execSqlQuery(query);
	
	query.prepare("delete from decklist_folders where id = :id");
	query.bindValue(":id", basePathId);
	servatrice->execSqlQuery(query);
}

ResponseCode ServerSocketInterface::cmdDeckDelDir(Command_DeckDelDir *cmd)
{
	int basePathId = getDeckPathId(cmd->getPath());
	if (basePathId == -1)
		return RespNameNotFound;
	deckDelDirHelper(basePathId);
}

ResponseCode ServerSocketInterface::cmdDeckDel(Command_DeckDel *cmd)
{
	QSqlQuery query;
	
	query.prepare("select id from decklist_files where id = :id and user = :user");
	query.bindValue(":id", cmd->getDeckId());
	query.bindValue(":user", playerName);
	servatrice->execSqlQuery(query);
	if (!query.next())
		return RespNameNotFound;
	
	query.prepare("delete from decklist_files where id = :id");
	query.bindValue(":id", cmd->getDeckId());
	servatrice->execSqlQuery(query);
	
	return RespOk;
}

ResponseCode ServerSocketInterface::cmdDeckUpload(Command_DeckUpload *cmd)
{
}

ResponseCode ServerSocketInterface::cmdDeckDownload(Command_DeckDownload *cmd)
{
}
