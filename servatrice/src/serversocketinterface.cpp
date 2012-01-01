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

#include <QtSql>
#include <QHostAddress>
#include <QDebug>
#include "serversocketinterface.h"
#include "servatrice.h"
#include "decklist.h"
#include "server_player.h"
#include "main.h"
#include "server_logger.h"
#include "server_response_containers.h"
#include "pb/commands.pb.h"
#include "pb/command_deck_upload.pb.h"
#include "pb/command_deck_download.pb.h"
#include "pb/command_deck_new_dir.pb.h"
#include "pb/command_deck_del_dir.pb.h"
#include "pb/command_deck_del.pb.h"
#include "pb/event_connection_closed.pb.h"
#include "pb/event_server_message.pb.h"
#include "pb/event_server_identification.pb.h"
#include "pb/event_add_to_list.pb.h"
#include "pb/event_remove_from_list.pb.h"
#include "pb/response_deck_list.pb.h"
#include "pb/response_deck_download.pb.h"
#include "pb/response_deck_upload.pb.h"
#include "pb/serverinfo_user.pb.h"
#include "pb/serverinfo_deckstorage.pb.h"

#include <string>
#include <iostream>

static const int protocolVersion = 13;

ServerSocketInterface::ServerSocketInterface(Servatrice *_server, QTcpSocket *_socket, QObject *parent)
	: Server_ProtocolHandler(_server, parent), servatrice(_server), socket(_socket), messageInProgress(false)
{
	connect(socket, SIGNAL(readyRead()), this, SLOT(readClient()));
	connect(socket, SIGNAL(disconnected()), this, SLOT(deleteLater()));
	connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(catchSocketError(QAbstractSocket::SocketError)));
	connect(this, SIGNAL(outputBufferChanged()), this, SLOT(flushOutputBuffer()), Qt::QueuedConnection);
	
	Event_ServerIdentification identEvent;
	identEvent.set_server_name(servatrice->getServerName().toStdString());
	identEvent.set_server_version(Servatrice::versionString.toStdString());
	identEvent.set_protocol_version(protocolVersion);
	SessionEvent *identSe = prepareSessionEvent(identEvent);
	sendProtocolItem(*identSe);
	delete identSe;
	
	int maxUsers = _server->getMaxUsersPerAddress();
	if ((maxUsers > 0) && (_server->getUsersWithAddress(socket->peerAddress()) >= maxUsers)) {
		Event_ConnectionClosed event;
		event.set_reason(Event_ConnectionClosed::TOO_MANY_CONNECTIONS);
		SessionEvent *se = prepareSessionEvent(event);
		sendProtocolItem(*se);
		delete se;
		
		deleteLater();
	}
	
	server->addClient(this);
}

ServerSocketInterface::~ServerSocketInterface()
{
	logger->logMessage("ServerSocketInterface destructor", this);
	
	prepareDestroy();
	
	flushOutputBuffer();
	delete socket;
	socket = 0;
}

void ServerSocketInterface::flushOutputBuffer()
{
	QMutexLocker locker(&outputBufferMutex);
	if (outputBuffer.isEmpty())
		return;
	servatrice->incTxBytes(outputBuffer.size());
	socket->write(outputBuffer);
	socket->flush();
	outputBuffer.clear();
}

void ServerSocketInterface::readClient()
{
	QByteArray data = socket->readAll();
	servatrice->incRxBytes(data.size());
	inputBuffer.append(data);
	
	do {
		if (!messageInProgress) {
			if (inputBuffer.size() >= 4) {
				messageLength =   (((quint32) (unsigned char) inputBuffer[0]) << 24)
				                + (((quint32) (unsigned char) inputBuffer[1]) << 16)
				                + (((quint32) (unsigned char) inputBuffer[2]) << 8)
				                + ((quint32) (unsigned char) inputBuffer[3]);
				inputBuffer.remove(0, 4);
				messageInProgress = true;
			} else
				return;
		}
		if (inputBuffer.size() < messageLength)
			return;
		
		CommandContainer newCommandContainer;
		newCommandContainer.ParseFromArray(inputBuffer.data(), messageLength);
		logger->logMessage(QString::fromStdString(newCommandContainer.ShortDebugString()), this);
		inputBuffer.remove(0, messageLength);
		messageInProgress = false;
		
		processCommandContainer(newCommandContainer);
	} while (!inputBuffer.isEmpty());
}

void ServerSocketInterface::catchSocketError(QAbstractSocket::SocketError socketError)
{
	qDebug() << "Socket error:" << socketError;
	
	deleteLater();
}

void ServerSocketInterface::transmitProtocolItem(const ServerMessage &item)
{
	QByteArray buf;
	unsigned int size = item.ByteSize();
	buf.resize(size + 4);
	item.SerializeToArray(buf.data() + 4, size);
	buf.data()[3] = (unsigned char) size;
	buf.data()[2] = (unsigned char) (size >> 8);
	buf.data()[1] = (unsigned char) (size >> 16);
	buf.data()[0] = (unsigned char) (size >> 24);
	
	QMutexLocker locker(&outputBufferMutex);
	outputBuffer.append(buf);
	emit outputBufferChanged();
}

Response::ResponseCode ServerSocketInterface::cmdAddToList(const Command_AddToList &cmd, ResponseContainer &rc)
{
	if (authState != PasswordRight)
		return Response::RespFunctionNotAllowed;
	
	QString list = QString::fromStdString(cmd.list());
	QString user = QString::fromStdString(cmd.user_name());
	
	if ((list != "buddy") && (list != "ignore"))
		return Response::RespContextError;
	
	if (list == "buddy")
		if (servatrice->isInBuddyList(QString::fromStdString(userInfo->name()), user))
			return Response::RespContextError;
	if (list == "ignore")
		if (servatrice->isInIgnoreList(QString::fromStdString(userInfo->name()), user))
			return Response::RespContextError;
	
	int id1 = servatrice->getUserIdInDB(QString::fromStdString(userInfo->name()));
	int id2 = servatrice->getUserIdInDB(user);
	if (id2 < 0)
		return Response::RespNameNotFound;
	if (id1 == id2)
		return Response::RespContextError;
	
	QMutexLocker locker(&servatrice->dbMutex);
	QSqlQuery query;
	query.prepare("insert into " + servatrice->getDbPrefix() + "_" + list + "list (id_user1, id_user2) values(:id1, :id2)");
	query.bindValue(":id1", id1);
	query.bindValue(":id2", id2);
	if (!servatrice->execSqlQuery(query))
		return Response::RespInternalError;
	
	Event_AddToList *event = new Event_AddToList;
	event->set_list_name(cmd.list());
	event->mutable_user_info()->CopyFrom(servatrice->getUserData(user));
	rc.enqueuePreResponseItem(ServerMessage::SESSION_EVENT, event);
	
	return Response::RespOk;
}

Response::ResponseCode ServerSocketInterface::cmdRemoveFromList(const Command_RemoveFromList &cmd, ResponseContainer &rc)
{
	if (authState != PasswordRight)
		return Response::RespFunctionNotAllowed;
	
	QString list = QString::fromStdString(cmd.list());
	QString user = QString::fromStdString(cmd.user_name());
	
	if ((list != "buddy") && (list != "ignore"))
		return Response::RespContextError;
	
	if (list == "buddy")
		if (!servatrice->isInBuddyList(QString::fromStdString(userInfo->name()), user))
			return Response::RespContextError;
	if (list == "ignore")
		if (!servatrice->isInIgnoreList(QString::fromStdString(userInfo->name()), user))
			return Response::RespContextError;
	
	int id1 = servatrice->getUserIdInDB(QString::fromStdString(userInfo->name()));
	int id2 = servatrice->getUserIdInDB(user);
	if (id2 < 0)
		return Response::RespNameNotFound;
	
	QMutexLocker locker(&servatrice->dbMutex);
	QSqlQuery query;
	query.prepare("delete from " + servatrice->getDbPrefix() + "_" + list + "list where id_user1 = :id1 and id_user2 = :id2");
	query.bindValue(":id1", id1);
	query.bindValue(":id2", id2);
	if (!servatrice->execSqlQuery(query))
		return Response::RespInternalError;
	
	Event_RemoveFromList *event = new Event_RemoveFromList;
	event->set_list_name(cmd.list());
	event->set_user_name(cmd.user_name());
	rc.enqueuePreResponseItem(ServerMessage::SESSION_EVENT, event);
	
	return Response::RespOk;
}

int ServerSocketInterface::getDeckPathId(int basePathId, QStringList path)
{
	if (path.isEmpty())
		return 0;
	if (path[0].isEmpty())
		return 0;
	
	QMutexLocker locker(&servatrice->dbMutex);
	QSqlQuery query;
	query.prepare("select id from " + servatrice->getDbPrefix() + "_decklist_folders where id_parent = :id_parent and name = :name and user = :user");
	query.bindValue(":id_parent", basePathId);
	query.bindValue(":name", path.takeFirst());
	query.bindValue(":user", QString::fromStdString(userInfo->name()));
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

bool ServerSocketInterface::deckListHelper(int folderId, ServerInfo_DeckStorage_Folder *folder)
{
	QMutexLocker locker(&servatrice->dbMutex);
	QSqlQuery query;
	query.prepare("select id, name from " + servatrice->getDbPrefix() + "_decklist_folders where id_parent = :id_parent and user = :user");
	query.bindValue(":id_parent", folderId);
	query.bindValue(":user", QString::fromStdString(userInfo->name()));
	if (!servatrice->execSqlQuery(query))
		return false;
	
	while (query.next()) {
		ServerInfo_DeckStorage_TreeItem *newItem = folder->add_items();
		newItem->set_id(query.value(0).toInt());
		newItem->set_name(query.value(1).toString().toStdString());
		
		if (!deckListHelper(newItem->id(), newItem->mutable_folder()))
			return false;
	}
	
	query.prepare("select id, name, upload_time from " + servatrice->getDbPrefix() + "_decklist_files where id_folder = :id_folder and user = :user");
	query.bindValue(":id_folder", folderId);
	query.bindValue(":user", QString::fromStdString(userInfo->name()));
	if (!servatrice->execSqlQuery(query))
		return false;
	
	while (query.next()) {
		ServerInfo_DeckStorage_TreeItem *newItem = folder->add_items();
		newItem->set_id(query.value(0).toInt());
		newItem->set_name(query.value(1).toString().toStdString());
		
		ServerInfo_DeckStorage_File *newFile = newItem->mutable_file();
		newFile->set_creation_time(query.value(2).toDateTime().toMSecsSinceEpoch());
	}
	
	return true;
}

// CHECK AUTHENTICATION!
// Also check for every function that data belonging to other users cannot be accessed.

Response::ResponseCode ServerSocketInterface::cmdDeckList(const Command_DeckList & /*cmd*/, ResponseContainer &rc)
{
	if (authState != PasswordRight)
		return Response::RespFunctionNotAllowed;
	
	servatrice->checkSql();
	
	Response_DeckList *re = new Response_DeckList;
	ServerInfo_DeckStorage_Folder *root = re->mutable_root();
	
	if (!deckListHelper(0, root))
		return Response::RespContextError;
	
	rc.setResponseExtension(re);
	return Response::RespOk;
}

Response::ResponseCode ServerSocketInterface::cmdDeckNewDir(const Command_DeckNewDir &cmd, ResponseContainer & /*rc*/)
{
	if (authState != PasswordRight)
		return Response::RespFunctionNotAllowed;
	
	servatrice->checkSql();
	
	int folderId = getDeckPathId(QString::fromStdString(cmd.path()));
	if (folderId == -1)
		return Response::RespNameNotFound;
	
	QMutexLocker locker(&servatrice->dbMutex);
	QSqlQuery query;
	query.prepare("insert into " + servatrice->getDbPrefix() + "_decklist_folders (id_parent, user, name) values(:id_parent, :user, :name)");
	query.bindValue(":id_parent", folderId);
	query.bindValue(":user", QString::fromStdString(userInfo->name()));
	query.bindValue(":name", QString::fromStdString(cmd.dir_name()));
	if (!servatrice->execSqlQuery(query))
		return Response::RespContextError;
	return Response::RespOk;
}

void ServerSocketInterface::deckDelDirHelper(int basePathId)
{
	servatrice->checkSql();
	
	QMutexLocker locker(&servatrice->dbMutex);
	QSqlQuery query;
	
	query.prepare("select id from " + servatrice->getDbPrefix() + "_decklist_folders where id_parent = :id_parent");
	query.bindValue(":id_parent", basePathId);
	servatrice->execSqlQuery(query);
	while (query.next())
		deckDelDirHelper(query.value(0).toInt());
	
	query.prepare("delete from " + servatrice->getDbPrefix() + "_decklist_files where id_folder = :id_folder");
	query.bindValue(":id_folder", basePathId);
	servatrice->execSqlQuery(query);
	
	query.prepare("delete from " + servatrice->getDbPrefix() + "_decklist_folders where id = :id");
	query.bindValue(":id", basePathId);
	servatrice->execSqlQuery(query);
}

Response::ResponseCode ServerSocketInterface::cmdDeckDelDir(const Command_DeckDelDir &cmd, ResponseContainer & /*rc*/)
{
	if (authState != PasswordRight)
		return Response::RespFunctionNotAllowed;
	
	servatrice->checkSql();
	
	int basePathId = getDeckPathId(QString::fromStdString(cmd.path()));
	if (basePathId == -1)
		return Response::RespNameNotFound;
	deckDelDirHelper(basePathId);
	return Response::RespOk;
}

Response::ResponseCode ServerSocketInterface::cmdDeckDel(const Command_DeckDel &cmd, ResponseContainer & /*rc*/)
{
	if (authState != PasswordRight)
		return Response::RespFunctionNotAllowed;
	
	servatrice->checkSql();
	
	QMutexLocker locker(&servatrice->dbMutex);
	QSqlQuery query;
	
	query.prepare("select id from " + servatrice->getDbPrefix() + "_decklist_files where id = :id and user = :user");
	query.bindValue(":id", cmd.deck_id());
	query.bindValue(":user", QString::fromStdString(userInfo->name()));
	servatrice->execSqlQuery(query);
	if (!query.next())
		return Response::RespNameNotFound;
	
	query.prepare("delete from " + servatrice->getDbPrefix() + "_decklist_files where id = :id");
	query.bindValue(":id", cmd.deck_id());
	servatrice->execSqlQuery(query);
	
	return Response::RespOk;
}

Response::ResponseCode ServerSocketInterface::cmdDeckUpload(const Command_DeckUpload &cmd, ResponseContainer &rc)
{
	if (authState != PasswordRight)
		return Response::RespFunctionNotAllowed;
	
	servatrice->checkSql();
	
	if (!cmd.has_deck_list())
		return Response::RespInvalidData;
	int folderId = getDeckPathId(QString::fromStdString(cmd.path()));
	if (folderId == -1)
		return Response::RespNameNotFound;
	
	QString deckStr = QString::fromStdString(cmd.deck_list());
	DeckList deck(deckStr);
	
	QString deckName = deck.getName();
	if (deckName.isEmpty())
		deckName = "Unnamed deck";

	QMutexLocker locker(&servatrice->dbMutex);
	QSqlQuery query;
	query.prepare("insert into " + servatrice->getDbPrefix() + "_decklist_files (id_folder, user, name, upload_time, content) values(:id_folder, :user, :name, NOW(), :content)");
	query.bindValue(":id_folder", folderId);
	query.bindValue(":user", QString::fromStdString(userInfo->name()));
	query.bindValue(":name", deckName);
	query.bindValue(":content", deckStr);
	servatrice->execSqlQuery(query);
	
	Response_DeckUpload *re = new Response_DeckUpload;
	ServerInfo_DeckStorage_TreeItem *fileInfo = re->mutable_new_file();
	fileInfo->set_id(query.lastInsertId().toInt());
	fileInfo->set_name(deckName.toStdString());
	fileInfo->mutable_file()->set_creation_time(QDateTime::currentMSecsSinceEpoch());
	rc.setResponseExtension(re);
	
	return Response::RespOk;
}

DeckList *ServerSocketInterface::getDeckFromDatabase(int deckId)
{
	servatrice->checkSql();
	
	QMutexLocker locker(&servatrice->dbMutex);
	QSqlQuery query;
	
	query.prepare("select content from " + servatrice->getDbPrefix() + "_decklist_files where id = :id and user = :user");
	query.bindValue(":id", deckId);
	query.bindValue(":user", QString::fromStdString(userInfo->name()));
	servatrice->execSqlQuery(query);
	if (!query.next())
		throw Response::RespNameNotFound;
	
	QXmlStreamReader deckReader(query.value(0).toString());
	DeckList *deck = new DeckList;
	deck->loadFromXml(&deckReader);
	
	return deck;
}

Response::ResponseCode ServerSocketInterface::cmdDeckDownload(const Command_DeckDownload &cmd, ResponseContainer &rc)
{
	if (authState != PasswordRight)
		return Response::RespFunctionNotAllowed;
	
	DeckList *deck;
	try {
		deck = getDeckFromDatabase(cmd.deck_id());
	} catch(Response::ResponseCode r) {
		return r;
	}
	
	Response_DeckDownload *re = new Response_DeckDownload;
	re->set_deck(deck->writeToString_Native().toStdString());
	rc.setResponseExtension(re);
	delete deck;
	
	return Response::RespOk;
}

// MODERATOR FUNCTIONS.
// May be called by admins and moderators. Permission is checked by the calling function.

Response::ResponseCode ServerSocketInterface::cmdBanFromServer(const Command_BanFromServer &cmd, ResponseContainer & /*rc*/)
{
	QString userName = QString::fromStdString(cmd.user_name());
	QString address = QString::fromStdString(cmd.address());
	int minutes = cmd.minutes();
	
	servatrice->dbMutex.lock();
	QSqlQuery query;
	query.prepare("insert into " + servatrice->getDbPrefix() + "_bans (user_name, ip_address, id_admin, time_from, minutes, reason, visible_reason) values(:user_name, :ip_address, :id_admin, NOW(), :minutes, :reason, :visible_reason)");
	query.bindValue(":user_name", userName);
	query.bindValue(":ip_address", address);
	query.bindValue(":id_admin", servatrice->getUserIdInDB(QString::fromStdString(userInfo->name())));
	query.bindValue(":minutes", minutes);
	query.bindValue(":reason", QString::fromStdString(cmd.reason()) + "\n");
	query.bindValue(":visible_reason", QString::fromStdString(cmd.visible_reason()) + "\n");
	servatrice->execSqlQuery(query);
	servatrice->dbMutex.unlock();
	
	ServerSocketInterface *user = static_cast<ServerSocketInterface *>(server->getUsers().value(userName));
	if (user) {
		Event_ConnectionClosed event;
		event.set_reason(Event_ConnectionClosed::BANNED);
		if (cmd.has_visible_reason())
			event.set_reason_str(cmd.visible_reason());
		SessionEvent *se = user->prepareSessionEvent(event);
		user->sendProtocolItem(*se);
		delete se;
		user->deleteLater();
	}
	
	return Response::RespOk;
}

// ADMIN FUNCTIONS.
// Permission is checked by the calling function.

Response::ResponseCode ServerSocketInterface::cmdUpdateServerMessage(const Command_UpdateServerMessage & /*cmd*/, ResponseContainer & /*rc*/)
{
	servatrice->updateLoginMessage();
	return Response::RespOk;
}

Response::ResponseCode ServerSocketInterface::cmdShutdownServer(const Command_ShutdownServer &cmd, ResponseContainer & /*rc*/)
{
	servatrice->scheduleShutdown(QString::fromStdString(cmd.reason()), cmd.minutes());
	return Response::RespOk;
}
