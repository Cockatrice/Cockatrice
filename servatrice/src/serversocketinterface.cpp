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
#include "pb/command_replay_list.pb.h"
#include "pb/command_replay_download.pb.h"
#include "pb/command_replay_modify_match.pb.h"
#include "pb/event_connection_closed.pb.h"
#include "pb/event_server_message.pb.h"
#include "pb/event_server_identification.pb.h"
#include "pb/event_add_to_list.pb.h"
#include "pb/event_remove_from_list.pb.h"
#include "pb/response_deck_list.pb.h"
#include "pb/response_deck_download.pb.h"
#include "pb/response_deck_upload.pb.h"
#include "pb/response_replay_list.pb.h"
#include "pb/response_replay_download.pb.h"
#include "pb/serverinfo_replay.pb.h"
#include "pb/serverinfo_user.pb.h"
#include "pb/serverinfo_deckstorage.pb.h"

#include "version_string.h"
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
	connect(this, SIGNAL(logDebugMessage(const QString &, Server_ProtocolHandler *)), logger, SLOT(logMessage(QString, Server_ProtocolHandler *)));
	
	Event_ServerIdentification identEvent;
	identEvent.set_server_name(servatrice->getServerName().toStdString());
	identEvent.set_server_version(VERSION_STRING);
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
	
	int id1 = userInfo->id();
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
	
	int id1 = userInfo->id();
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
		newFile->set_creation_time(query.value(2).toDateTime().toTime_t());
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
	fileInfo->mutable_file()->set_creation_time(QDateTime::currentDateTime().toTime_t());
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

Response::ResponseCode ServerSocketInterface::cmdReplayList(const Command_ReplayList & /*cmd*/, ResponseContainer &rc)
{
	if (authState != PasswordRight)
		return Response::RespFunctionNotAllowed;
	
	Response_ReplayList *re = new Response_ReplayList;
	
	servatrice->dbMutex.lock();
	QSqlQuery query1;
	query1.prepare("select a.id_game, a.replay_name, b.room_name, b.time_started, b.time_finished, b.descr, a.do_not_hide from cockatrice_replays_access a left join cockatrice_games b on b.id = a.id_game where a.id_player = :id_player and (a.do_not_hide = 1 or date_add(b.time_started, interval 14 day) > now())");
	query1.bindValue(":id_player", userInfo->id());
	servatrice->execSqlQuery(query1);
	while (query1.next()) {
		ServerInfo_ReplayMatch *matchInfo = re->add_match_list();
		
		const int gameId = query1.value(0).toInt();
		matchInfo->set_game_id(gameId);
		matchInfo->set_room_name(query1.value(2).toString().toStdString());
		const int timeStarted = query1.value(3).toDateTime().toTime_t();
		const int timeFinished = query1.value(4).toDateTime().toTime_t();
		matchInfo->set_time_started(timeStarted);
		matchInfo->set_length(timeFinished - timeStarted);
		matchInfo->set_game_name(query1.value(5).toString().toStdString());
		const QString replayName = query1.value(1).toString();
		matchInfo->set_do_not_hide(query1.value(6).toBool());
		
		QSqlQuery query2;
		query2.prepare("select player_name from cockatrice_games_players where id_game = :id_game");
		query2.bindValue(":id_game", gameId);
		servatrice->execSqlQuery(query2);
		while (query2.next())
			matchInfo->add_player_names(query2.value(0).toString().toStdString());
		
		QSqlQuery query3;
		query3.prepare("select id, duration from " + servatrice->getDbPrefix() + "_replays where id_game = :id_game");
		query3.bindValue(":id_game", gameId);
		servatrice->execSqlQuery(query3);
		while (query3.next()) {
			ServerInfo_Replay *replayInfo = matchInfo->add_replay_list();
			replayInfo->set_replay_id(query3.value(0).toInt());
			replayInfo->set_replay_name(replayName.toStdString());
			replayInfo->set_duration(query3.value(1).toInt());
		}
	}
	servatrice->dbMutex.unlock();
	
	rc.setResponseExtension(re);
	return Response::RespOk;
}

Response::ResponseCode ServerSocketInterface::cmdReplayDownload(const Command_ReplayDownload &cmd, ResponseContainer &rc)
{
	if (authState != PasswordRight)
		return Response::RespFunctionNotAllowed;
	
	QMutexLocker dbLocker(&servatrice->dbMutex);
	
	QSqlQuery query1;
	query1.prepare("select 1 from " + servatrice->getDbPrefix() + "_replays_access a left join " + servatrice->getDbPrefix() + "_replays b on a.id_game = b.id_game where b.id = :id_replay and a.id_player = :id_player");
	query1.bindValue(":id_replay", cmd.replay_id());
	query1.bindValue(":id_player", userInfo->id());
	if (!servatrice->execSqlQuery(query1))
		return Response::RespInternalError;
	if (!query1.next())
		return Response::RespAccessDenied;
	
	QSqlQuery query2;
	query2.prepare("select replay from " + servatrice->getDbPrefix() + "_replays where id = :id_replay");
	query2.bindValue(":id_replay", cmd.replay_id());
	if (!servatrice->execSqlQuery(query2))
		return Response::RespInternalError;
	if (!query2.next())
		return Response::RespNameNotFound;
	
	QByteArray data = query2.value(0).toByteArray();
	
	Response_ReplayDownload *re = new Response_ReplayDownload;
	re->set_replay_data(data.data(), data.size());
	rc.setResponseExtension(re);
	
	return Response::RespOk;
}

Response::ResponseCode ServerSocketInterface::cmdReplayModifyMatch(const Command_ReplayModifyMatch &cmd, ResponseContainer & /*rc*/)
{
	if (authState != PasswordRight)
		return Response::RespFunctionNotAllowed;
	
	QMutexLocker dbLocker(&servatrice->dbMutex);
	
	QSqlQuery query1;
	query1.prepare("update " + servatrice->getDbPrefix() + "_replays_access set do_not_hide=:do_not_hide where id_player = :id_player and id_game = :id_game");
	query1.bindValue(":id_player", userInfo->id());
	query1.bindValue(":id_game", cmd.game_id());
	query1.bindValue(":do_not_hide", cmd.do_not_hide());
	
	return servatrice->execSqlQuery(query1) ? Response::RespOk : Response::RespNameNotFound;
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
	query.bindValue(":id_admin", userInfo->id());
	query.bindValue(":minutes", minutes);
	query.bindValue(":reason", QString::fromStdString(cmd.reason()) + "\n");
	query.bindValue(":visible_reason", QString::fromStdString(cmd.visible_reason()) + "\n");
	servatrice->execSqlQuery(query);
	servatrice->dbMutex.unlock();
	
	QList<ServerSocketInterface *> userList = servatrice->getUsersWithAddressAsList(QHostAddress(address));
	ServerSocketInterface *user = static_cast<ServerSocketInterface *>(server->getUsers().value(userName));
	if (user && !userList.contains(user))
		userList.append(user);
	if (!userList.isEmpty()) {
		Event_ConnectionClosed event;
		event.set_reason(Event_ConnectionClosed::BANNED);
		if (cmd.has_visible_reason())
			event.set_reason_str(cmd.visible_reason());
		for (int i = 0; i < userList.size(); ++i) {
			SessionEvent *se = userList[i]->prepareSessionEvent(event);
			userList[i]->sendProtocolItem(*se);
			userList[i]->deleteLater();
			delete se;
		}
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
