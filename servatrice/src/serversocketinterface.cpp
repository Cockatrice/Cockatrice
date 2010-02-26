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
#include <QDebug>
#include "serversocketinterface.h"
#include "servatrice.h"
#include "protocol.h"
#include "protocol_items.h"
#include "decklist.h"
#include "server_player.h"

ServerSocketInterface::ServerSocketInterface(Servatrice *_server, QTcpSocket *_socket, QObject *parent)
	: Server_ProtocolHandler(_server, parent), servatrice(_server), socket(_socket), topLevelItem(0)
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

void ServerSocketInterface::processProtocolItem(ProtocolItem *item)
{
	CommandContainer *cont = qobject_cast<CommandContainer *>(item);
	if (!cont)
		sendProtocolItem(new ProtocolResponse(cont->getCmdId(), RespInvalidCommand));
	else
		processCommandContainer(cont);
}

void ServerSocketInterface::readClient()
{
	QByteArray data = socket->readAll();
	qDebug() << data;
	xmlReader->addData(data);
	
	while (!xmlReader->atEnd()) {
		xmlReader->readNext();
		if (topLevelItem)
			topLevelItem->readElement(xmlReader);
		else if (xmlReader->isStartElement() && (xmlReader->name().toString() == "cockatrice_client_stream")) {
			topLevelItem = new TopLevelProtocolItem;
			connect(topLevelItem, SIGNAL(protocolItemReceived(ProtocolItem *)), this, SLOT(processProtocolItem(ProtocolItem *)));
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

bool ServerSocketInterface::deckListHelper(DeckList_Directory *folder)
{
	QSqlQuery query;
	query.prepare("select id, name from decklist_folders where id_parent = :id_parent and user = :user");
	query.bindValue(":id_parent", folder->getId());
	query.bindValue(":user", playerName);
	if (!servatrice->execSqlQuery(query))
		return false;
	
	while (query.next()) {
		DeckList_Directory *newFolder = new DeckList_Directory(query.value(1).toString(), query.value(0).toInt());
		folder->appendItem(newFolder);
		if (!deckListHelper(newFolder))
			return false;
	}
	
	query.prepare("select id, name, upload_time from decklist_files where id_folder = :id_folder and user = :user");
	query.bindValue(":id_folder", folder->getId());
	query.bindValue(":user", playerName);
	if (!servatrice->execSqlQuery(query))
		return false;
	
	while (query.next()) {
		DeckList_File *newFile = new DeckList_File(query.value(1).toString(), query.value(0).toInt(), query.value(2).toDateTime());
		folder->appendItem(newFile);
	}
	
	return true;
}

// CHECK AUTHENTICATION!
// Also check for every function that data belonging to other users cannot be accessed.

ResponseCode ServerSocketInterface::cmdDeckList(Command_DeckList * /*cmd*/, CommandContainer *cont)
{
	if (authState != PasswordRight)
		return RespFunctionNotAllowed;
	
	servatrice->checkSql();
	
	DeckList_Directory *root = new DeckList_Directory(QString());
	QSqlQuery query;
	if (!deckListHelper(root))
		return RespContextError;
	
	cont->setResponse(new Response_DeckList(cont->getCmdId(), RespOk, root));
	
	return RespNothing;
}

ResponseCode ServerSocketInterface::cmdDeckNewDir(Command_DeckNewDir *cmd, CommandContainer * /*cont*/)
{
	if (authState != PasswordRight)
		return RespFunctionNotAllowed;
	
	servatrice->checkSql();
	
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
	servatrice->checkSql();
	
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

ResponseCode ServerSocketInterface::cmdDeckDelDir(Command_DeckDelDir *cmd, CommandContainer * /*cont*/)
{
	if (authState != PasswordRight)
		return RespFunctionNotAllowed;
	
	servatrice->checkSql();
	
	int basePathId = getDeckPathId(cmd->getPath());
	if (basePathId == -1)
		return RespNameNotFound;
	deckDelDirHelper(basePathId);
	return RespOk;
}

ResponseCode ServerSocketInterface::cmdDeckDel(Command_DeckDel *cmd, CommandContainer * /*cont*/)
{
	if (authState != PasswordRight)
		return RespFunctionNotAllowed;
	
	servatrice->checkSql();
	
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

ResponseCode ServerSocketInterface::cmdDeckUpload(Command_DeckUpload *cmd, CommandContainer *cont)
{
	if (authState != PasswordRight)
		return RespFunctionNotAllowed;
	
	servatrice->checkSql();
	
	if (!cmd->getDeck())
		return RespInvalidData;
	int folderId = getDeckPathId(cmd->getPath());
	if (folderId == -1)
		return RespNameNotFound;
	
	QString deckContents;
	QXmlStreamWriter deckWriter(&deckContents);
	deckWriter.writeStartDocument();
	cmd->getDeck()->write(&deckWriter);
	deckWriter.writeEndDocument();
	
	QString deckName = cmd->getDeck()->getName();
	if (deckName.isEmpty())
		deckName = "Unnamed deck";

	QSqlQuery query;
	query.prepare("insert into decklist_files (id_folder, user, name, upload_time, content) values(:id_folder, :user, :name, NOW(), :content)");
	query.bindValue(":id_folder", folderId);
	query.bindValue(":user", playerName);
	query.bindValue(":name", deckName);
	query.bindValue(":content", deckContents);
	servatrice->execSqlQuery(query);
	
	cont->setResponse(new Response_DeckUpload(cont->getCmdId(), RespOk, new DeckList_File(deckName, query.lastInsertId().toInt(), QDateTime::currentDateTime())));
	return RespNothing;
}

DeckList *ServerSocketInterface::getDeckFromDatabase(int deckId)
{
	servatrice->checkSql();
	
	QSqlQuery query;
	
	query.prepare("select content from decklist_files where id = :id and user = :user");
	query.bindValue(":id", deckId);
	query.bindValue(":user", playerName);
	servatrice->execSqlQuery(query);
	if (!query.next())
		throw RespNameNotFound;
	
	QXmlStreamReader deckReader(query.value(0).toString());
	DeckList *deck = new DeckList;
	deck->loadFromXml(&deckReader);
	
	return deck;
}

ResponseCode ServerSocketInterface::cmdDeckDownload(Command_DeckDownload *cmd, CommandContainer *cont)
{
	if (authState != PasswordRight)
		return RespFunctionNotAllowed;
	
	DeckList *deck;
	try {
		deck = getDeckFromDatabase(cmd->getDeckId());
	} catch(ResponseCode r) {
		return r;
	}
	cont->setResponse(new Response_DeckDownload(cont->getCmdId(), RespOk, deck));
	return RespNothing;
}
