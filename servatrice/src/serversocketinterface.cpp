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

#include <QSqlQuery>
#include <QHostAddress>
#include <QDebug>
#include <QDateTime>
#include "serversocketinterface.h"
#include "servatrice.h"
#include "servatrice_database_interface.h"
#include "decklist.h"
#include "server_player.h"
#include "main.h"
#include "server_logger.h"
#include "server_response_containers.h"
#include "pb/commands.pb.h"
#include "pb/command_deck_list.pb.h"
#include "pb/command_deck_upload.pb.h"
#include "pb/command_deck_download.pb.h"
#include "pb/command_deck_new_dir.pb.h"
#include "pb/command_deck_del_dir.pb.h"
#include "pb/command_deck_del.pb.h"
#include "pb/command_replay_list.pb.h"
#include "pb/command_replay_download.pb.h"
#include "pb/command_replay_modify_match.pb.h"
#include "pb/command_replay_delete_match.pb.h"
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

static const int protocolVersion = 14;

ServerSocketInterface::ServerSocketInterface(Servatrice *_server, Servatrice_DatabaseInterface *_databaseInterface, QObject *parent)
	: Server_ProtocolHandler(_server, _databaseInterface, parent),
	  servatrice(_server),
	  sqlInterface(reinterpret_cast<Servatrice_DatabaseInterface *>(databaseInterface)),
	  messageInProgress(false),
	  handshakeStarted(false)
{
	socket = new QTcpSocket(this);
	socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
	connect(socket, SIGNAL(readyRead()), this, SLOT(readClient()));
	connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(catchSocketError(QAbstractSocket::SocketError)));
	
	// Never call flushOutputQueue directly from outputQueueChanged. In case of a socket error,
	// it could lead to this object being destroyed while another function is still on the call stack. -> mutex deadlocks etc.
	connect(this, SIGNAL(outputQueueChanged()), this, SLOT(flushOutputQueue()), Qt::QueuedConnection);
}

ServerSocketInterface::~ServerSocketInterface()
{
	logger->logMessage("ServerSocketInterface destructor", this);
	
	flushOutputQueue();
}

void ServerSocketInterface::initConnection(int socketDescriptor)
{
	// Add this object to the server's list of connections before it can receive socket events.
	// Otherwise, in case a of a socket error, it could be removed from the list before it is added.
	server->addClient(this);
	
	socket->setSocketDescriptor(socketDescriptor);
	logger->logMessage(QString("Incoming connection: %1").arg(socket->peerAddress().toString()), this);
	initSessionDeprecated();
}

void ServerSocketInterface::initSessionDeprecated()
{
	// dirty hack to make v13 client display the correct error message
	
	QByteArray buf;
	buf.append("<?xml version=\"1.0\"?><cockatrice_server_stream version=\"14\">");
	socket->write(buf);
	socket->flush();
}

bool ServerSocketInterface::initSession()
{
	Event_ServerIdentification identEvent;
	identEvent.set_server_name(servatrice->getServerName().toStdString());
	identEvent.set_server_version(VERSION_STRING);
	identEvent.set_protocol_version(protocolVersion);
	SessionEvent *identSe = prepareSessionEvent(identEvent);
	sendProtocolItem(*identSe);
	delete identSe;
	
	int maxUsers = servatrice->getMaxUsersPerAddress();
	if ((maxUsers > 0) && (servatrice->getUsersWithAddress(socket->peerAddress()) >= maxUsers)) {
		Event_ConnectionClosed event;
		event.set_reason(Event_ConnectionClosed::TOO_MANY_CONNECTIONS);
		SessionEvent *se = prepareSessionEvent(event);
		sendProtocolItem(*se);
		delete se;
		
		return false;
	}
	
	return true;
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
		
		// dirty hack to make v13 client display the correct error message
		if (handshakeStarted)
			processCommandContainer(newCommandContainer);
		else if (!newCommandContainer.has_cmd_id()) {
			handshakeStarted = true;
			if (!initSession())
				prepareDestroy();
		}
		// end of hack
	} while (!inputBuffer.isEmpty());
}

void ServerSocketInterface::catchSocketError(QAbstractSocket::SocketError socketError)
{
	qDebug() << "Socket error:" << socketError;
	
	prepareDestroy();
}

void ServerSocketInterface::transmitProtocolItem(const ServerMessage &item)
{
	outputQueueMutex.lock();
	outputQueue.append(item);
	outputQueueMutex.unlock();
	
	emit outputQueueChanged();
}

void ServerSocketInterface::flushOutputQueue()
{
	QMutexLocker locker(&outputQueueMutex);
	if (outputQueue.isEmpty())
		return;
	
	int totalBytes = 0;
	while (!outputQueue.isEmpty()) {
		ServerMessage item = outputQueue.takeFirst();
		locker.unlock();
		
		QByteArray buf;
		unsigned int size = item.ByteSize();
		buf.resize(size + 4);
		item.SerializeToArray(buf.data() + 4, size);
		buf.data()[3] = (unsigned char) size;
		buf.data()[2] = (unsigned char) (size >> 8);
		buf.data()[1] = (unsigned char) (size >> 16);
		buf.data()[0] = (unsigned char) (size >> 24);
		// In case socket->write() calls catchSocketError(), the mutex must not be locked during this call.
		socket->write(buf);
		
		totalBytes += size + 4;
		locker.relock();
	}
	locker.unlock();
	servatrice->incTxBytes(totalBytes);
	// see above wrt mutex
	socket->flush();
}

void ServerSocketInterface::logDebugMessage(const QString &message)
{
	logger->logMessage(message, this);
}

Response::ResponseCode ServerSocketInterface::processExtendedSessionCommand(int cmdType, const SessionCommand &cmd, ResponseContainer &rc)
{
	switch ((SessionCommand::SessionCommandType) cmdType) {
		case SessionCommand::ADD_TO_LIST: return cmdAddToList(cmd.GetExtension(Command_AddToList::ext), rc);
		case SessionCommand::REMOVE_FROM_LIST: return cmdRemoveFromList(cmd.GetExtension(Command_RemoveFromList::ext), rc);
		case SessionCommand::DECK_LIST: return cmdDeckList(cmd.GetExtension(Command_DeckList::ext), rc);
		case SessionCommand::DECK_NEW_DIR: return cmdDeckNewDir(cmd.GetExtension(Command_DeckNewDir::ext), rc);
		case SessionCommand::DECK_DEL_DIR: return cmdDeckDelDir(cmd.GetExtension(Command_DeckDelDir::ext), rc);
		case SessionCommand::DECK_DEL: return cmdDeckDel(cmd.GetExtension(Command_DeckDel::ext), rc);
		case SessionCommand::DECK_UPLOAD: return cmdDeckUpload(cmd.GetExtension(Command_DeckUpload::ext), rc);
		case SessionCommand::DECK_DOWNLOAD: return cmdDeckDownload(cmd.GetExtension(Command_DeckDownload::ext), rc);
		case SessionCommand::REPLAY_LIST: return cmdReplayList(cmd.GetExtension(Command_ReplayList::ext), rc);
		case SessionCommand::REPLAY_DOWNLOAD: return cmdReplayDownload(cmd.GetExtension(Command_ReplayDownload::ext), rc);
		case SessionCommand::REPLAY_MODIFY_MATCH: return cmdReplayModifyMatch(cmd.GetExtension(Command_ReplayModifyMatch::ext), rc);
		case SessionCommand::REPLAY_DELETE_MATCH: return cmdReplayDeleteMatch(cmd.GetExtension(Command_ReplayDeleteMatch::ext), rc);
		default: return Response::RespFunctionNotAllowed;
	}
}

Response::ResponseCode ServerSocketInterface::processExtendedModeratorCommand(int cmdType, const ModeratorCommand &cmd, ResponseContainer &rc)
{
	switch ((ModeratorCommand::ModeratorCommandType) cmdType) {
		case ModeratorCommand::BAN_FROM_SERVER: return cmdBanFromServer(cmd.GetExtension(Command_BanFromServer::ext), rc);
		default: return Response::RespFunctionNotAllowed;
	}
}

Response::ResponseCode ServerSocketInterface::processExtendedAdminCommand(int cmdType, const AdminCommand &cmd, ResponseContainer &rc)
{
	switch ((AdminCommand::AdminCommandType) cmdType) {
		case AdminCommand::SHUTDOWN_SERVER: return cmdShutdownServer(cmd.GetExtension(Command_ShutdownServer::ext), rc);
		case AdminCommand::UPDATE_SERVER_MESSAGE: return cmdUpdateServerMessage(cmd.GetExtension(Command_UpdateServerMessage::ext), rc);
		default: return Response::RespFunctionNotAllowed;
	}
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
		if (databaseInterface->isInBuddyList(QString::fromStdString(userInfo->name()), user))
			return Response::RespContextError;
	if (list == "ignore")
		if (databaseInterface->isInIgnoreList(QString::fromStdString(userInfo->name()), user))
			return Response::RespContextError;
	
	int id1 = userInfo->id();
	int id2 = sqlInterface->getUserIdInDB(user);
	if (id2 < 0)
		return Response::RespNameNotFound;
	if (id1 == id2)
		return Response::RespContextError;
	
	QSqlQuery query(sqlInterface->getDatabase());
	query.prepare("insert into " + servatrice->getDbPrefix() + "_" + list + "list (id_user1, id_user2) values(:id1, :id2)");
	query.bindValue(":id1", id1);
	query.bindValue(":id2", id2);
	if (!sqlInterface->execSqlQuery(query))
		return Response::RespInternalError;
	
	Event_AddToList event;
	event.set_list_name(cmd.list());
	event.mutable_user_info()->CopyFrom(databaseInterface->getUserData(user));
	rc.enqueuePreResponseItem(ServerMessage::SESSION_EVENT, prepareSessionEvent(event));
	
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
		if (!databaseInterface->isInBuddyList(QString::fromStdString(userInfo->name()), user))
			return Response::RespContextError;
	if (list == "ignore")
		if (!databaseInterface->isInIgnoreList(QString::fromStdString(userInfo->name()), user))
			return Response::RespContextError;
	
	int id1 = userInfo->id();
	int id2 = sqlInterface->getUserIdInDB(user);
	if (id2 < 0)
		return Response::RespNameNotFound;
	
	QSqlQuery query(sqlInterface->getDatabase());
	query.prepare("delete from " + servatrice->getDbPrefix() + "_" + list + "list where id_user1 = :id1 and id_user2 = :id2");
	query.bindValue(":id1", id1);
	query.bindValue(":id2", id2);
	if (!sqlInterface->execSqlQuery(query))
		return Response::RespInternalError;
	
	Event_RemoveFromList event;
	event.set_list_name(cmd.list());
	event.set_user_name(cmd.user_name());
	rc.enqueuePreResponseItem(ServerMessage::SESSION_EVENT, prepareSessionEvent(event));
	
	return Response::RespOk;
}

int ServerSocketInterface::getDeckPathId(int basePathId, QStringList path)
{
	if (path.isEmpty())
		return 0;
	if (path[0].isEmpty())
		return 0;
	
	QSqlQuery query(sqlInterface->getDatabase());
	query.prepare("select id from " + servatrice->getDbPrefix() + "_decklist_folders where id_parent = :id_parent and name = :name and id_user = :id_user");
	query.bindValue(":id_parent", basePathId);
	query.bindValue(":name", path.takeFirst());
	query.bindValue(":id_user", userInfo->id());
	if (!sqlInterface->execSqlQuery(query))
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
	QSqlQuery query(sqlInterface->getDatabase());
	query.prepare("select id, name from " + servatrice->getDbPrefix() + "_decklist_folders where id_parent = :id_parent and id_user = :id_user");
	query.bindValue(":id_parent", folderId);
	query.bindValue(":id_user", userInfo->id());
	if (!sqlInterface->execSqlQuery(query))
		return false;
	
	while (query.next()) {
		ServerInfo_DeckStorage_TreeItem *newItem = folder->add_items();
		newItem->set_id(query.value(0).toInt());
		newItem->set_name(query.value(1).toString().toStdString());
		
		if (!deckListHelper(newItem->id(), newItem->mutable_folder()))
			return false;
	}
	
	query.prepare("select id, name, upload_time from " + servatrice->getDbPrefix() + "_decklist_files where id_folder = :id_folder and id_user = :id_user");
	query.bindValue(":id_folder", folderId);
	query.bindValue(":id_user", userInfo->id());
	if (!sqlInterface->execSqlQuery(query))
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
	
	sqlInterface->checkSql();
	
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
	
	sqlInterface->checkSql();
	
	int folderId = getDeckPathId(QString::fromStdString(cmd.path()));
	if (folderId == -1)
		return Response::RespNameNotFound;
	
	QSqlQuery query(sqlInterface->getDatabase());
	query.prepare("insert into " + servatrice->getDbPrefix() + "_decklist_folders (id_parent, id_user, name) values(:id_parent, :id_user, :name)");
	query.bindValue(":id_parent", folderId);
	query.bindValue(":id_user", userInfo->id());
	query.bindValue(":name", QString::fromStdString(cmd.dir_name()));
	if (!sqlInterface->execSqlQuery(query))
		return Response::RespContextError;
	return Response::RespOk;
}

void ServerSocketInterface::deckDelDirHelper(int basePathId)
{
	sqlInterface->checkSql();
	QSqlQuery query(sqlInterface->getDatabase());
	
	query.prepare("select id from " + servatrice->getDbPrefix() + "_decklist_folders where id_parent = :id_parent");
	query.bindValue(":id_parent", basePathId);
	sqlInterface->execSqlQuery(query);
	while (query.next())
		deckDelDirHelper(query.value(0).toInt());
	
	query.prepare("delete from " + servatrice->getDbPrefix() + "_decklist_files where id_folder = :id_folder");
	query.bindValue(":id_folder", basePathId);
	sqlInterface->execSqlQuery(query);
	
	query.prepare("delete from " + servatrice->getDbPrefix() + "_decklist_folders where id = :id");
	query.bindValue(":id", basePathId);
	sqlInterface->execSqlQuery(query);
}

Response::ResponseCode ServerSocketInterface::cmdDeckDelDir(const Command_DeckDelDir &cmd, ResponseContainer & /*rc*/)
{
	if (authState != PasswordRight)
		return Response::RespFunctionNotAllowed;
	
	sqlInterface->checkSql();
	
	int basePathId = getDeckPathId(QString::fromStdString(cmd.path()));
	if ((basePathId == -1) || (basePathId == 0))
		return Response::RespNameNotFound;
	deckDelDirHelper(basePathId);
	return Response::RespOk;
}

Response::ResponseCode ServerSocketInterface::cmdDeckDel(const Command_DeckDel &cmd, ResponseContainer & /*rc*/)
{
	if (authState != PasswordRight)
		return Response::RespFunctionNotAllowed;
	
	sqlInterface->checkSql();
	QSqlQuery query(sqlInterface->getDatabase());
	
	query.prepare("select id from " + servatrice->getDbPrefix() + "_decklist_files where id = :id and id_user = :id_user");
	query.bindValue(":id", cmd.deck_id());
	query.bindValue(":id_user", userInfo->id());
	sqlInterface->execSqlQuery(query);
	if (!query.next())
		return Response::RespNameNotFound;
	
	query.prepare("delete from " + servatrice->getDbPrefix() + "_decklist_files where id = :id");
	query.bindValue(":id", cmd.deck_id());
	sqlInterface->execSqlQuery(query);
	
	return Response::RespOk;
}

Response::ResponseCode ServerSocketInterface::cmdDeckUpload(const Command_DeckUpload &cmd, ResponseContainer &rc)
{
	if (authState != PasswordRight)
		return Response::RespFunctionNotAllowed;
	
	if (!cmd.has_deck_list())
		return Response::RespInvalidData;
	
	sqlInterface->checkSql();
	
	QString deckStr = QString::fromStdString(cmd.deck_list());
	DeckList deck(deckStr);
	
	QString deckName = deck.getName();
	if (deckName.isEmpty())
		deckName = "Unnamed deck";
	
	if (cmd.has_path()) {
		int folderId = getDeckPathId(QString::fromStdString(cmd.path()));
		if (folderId == -1)
			return Response::RespNameNotFound;
		
		QSqlQuery query(sqlInterface->getDatabase());
		query.prepare("insert into " + servatrice->getDbPrefix() + "_decklist_files (id_folder, id_user, name, upload_time, content) values(:id_folder, :id_user, :name, NOW(), :content)");
		query.bindValue(":id_folder", folderId);
		query.bindValue(":id_user", userInfo->id());
		query.bindValue(":name", deckName);
		query.bindValue(":content", deckStr);
		sqlInterface->execSqlQuery(query);
		
		Response_DeckUpload *re = new Response_DeckUpload;
		ServerInfo_DeckStorage_TreeItem *fileInfo = re->mutable_new_file();
		fileInfo->set_id(query.lastInsertId().toInt());
		fileInfo->set_name(deckName.toStdString());
		fileInfo->mutable_file()->set_creation_time(QDateTime::currentDateTime().toTime_t());
		rc.setResponseExtension(re);
	} else if (cmd.has_deck_id()) {
		QSqlQuery query(sqlInterface->getDatabase());
		query.prepare("update " + servatrice->getDbPrefix() + "_decklist_files set name=:name, upload_time=NOW(), content=:content where id = :id_deck and id_user = :id_user");
		query.bindValue(":id_deck", cmd.deck_id());
		query.bindValue(":id_user", userInfo->id());
		query.bindValue(":name", deckName);
		query.bindValue(":content", deckStr);
		sqlInterface->execSqlQuery(query);
		
		if (query.numRowsAffected() == 0)
			return Response::RespNameNotFound;
		
		Response_DeckUpload *re = new Response_DeckUpload;
		ServerInfo_DeckStorage_TreeItem *fileInfo = re->mutable_new_file();
		fileInfo->set_id(cmd.deck_id());
		fileInfo->set_name(deckName.toStdString());
		fileInfo->mutable_file()->set_creation_time(QDateTime::currentDateTime().toTime_t());
		rc.setResponseExtension(re);
	} else
		return Response::RespInvalidData;
	
	return Response::RespOk;
}

Response::ResponseCode ServerSocketInterface::cmdDeckDownload(const Command_DeckDownload &cmd, ResponseContainer &rc)
{
	if (authState != PasswordRight)
		return Response::RespFunctionNotAllowed;
	
	DeckList *deck;
	try {
		deck = sqlInterface->getDeckFromDatabase(cmd.deck_id(), userInfo->id());
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
	
	QSqlQuery query1(sqlInterface->getDatabase());
	query1.prepare("select a.id_game, a.replay_name, b.room_name, b.time_started, b.time_finished, b.descr, a.do_not_hide from cockatrice_replays_access a left join cockatrice_games b on b.id = a.id_game where a.id_player = :id_player and (a.do_not_hide = 1 or date_add(b.time_started, interval 7 day) > now())");
	query1.bindValue(":id_player", userInfo->id());
	sqlInterface->execSqlQuery(query1);
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
		
		{
			QSqlQuery query2(sqlInterface->getDatabase());
			query2.prepare("select player_name from cockatrice_games_players where id_game = :id_game");
			query2.bindValue(":id_game", gameId);
			sqlInterface->execSqlQuery(query2);
			while (query2.next())
				matchInfo->add_player_names(query2.value(0).toString().toStdString());
		}
		{
			QSqlQuery query3(sqlInterface->getDatabase());
			query3.prepare("select id, duration from " + servatrice->getDbPrefix() + "_replays where id_game = :id_game");
			query3.bindValue(":id_game", gameId);
			sqlInterface->execSqlQuery(query3);
			while (query3.next()) {
				ServerInfo_Replay *replayInfo = matchInfo->add_replay_list();
				replayInfo->set_replay_id(query3.value(0).toInt());
				replayInfo->set_replay_name(replayName.toStdString());
				replayInfo->set_duration(query3.value(1).toInt());
			}
		}
	}
	
	rc.setResponseExtension(re);
	return Response::RespOk;
}

Response::ResponseCode ServerSocketInterface::cmdReplayDownload(const Command_ReplayDownload &cmd, ResponseContainer &rc)
{
	if (authState != PasswordRight)
		return Response::RespFunctionNotAllowed;
	
	{
		QSqlQuery query(sqlInterface->getDatabase());
		query.prepare("select 1 from " + servatrice->getDbPrefix() + "_replays_access a left join " + servatrice->getDbPrefix() + "_replays b on a.id_game = b.id_game where b.id = :id_replay and a.id_player = :id_player");
		query.bindValue(":id_replay", cmd.replay_id());
		query.bindValue(":id_player", userInfo->id());
		if (!sqlInterface->execSqlQuery(query))
			return Response::RespInternalError;
		if (!query.next())
			return Response::RespAccessDenied;
	}
	
	QSqlQuery query(sqlInterface->getDatabase());
	query.prepare("select replay from " + servatrice->getDbPrefix() + "_replays where id = :id_replay");
	query.bindValue(":id_replay", cmd.replay_id());
	if (!sqlInterface->execSqlQuery(query))
		return Response::RespInternalError;
	if (!query.next())
		return Response::RespNameNotFound;
	
	QByteArray data = query.value(0).toByteArray();
	
	Response_ReplayDownload *re = new Response_ReplayDownload;
	re->set_replay_data(data.data(), data.size());
	rc.setResponseExtension(re);
	
	return Response::RespOk;
}

Response::ResponseCode ServerSocketInterface::cmdReplayModifyMatch(const Command_ReplayModifyMatch &cmd, ResponseContainer & /*rc*/)
{
	if (authState != PasswordRight)
		return Response::RespFunctionNotAllowed;
	
	if (!sqlInterface->checkSql())
		return Response::RespInternalError;
	
	QSqlQuery query(sqlInterface->getDatabase());
	query.prepare("update " + servatrice->getDbPrefix() + "_replays_access set do_not_hide=:do_not_hide where id_player = :id_player and id_game = :id_game");
	query.bindValue(":id_player", userInfo->id());
	query.bindValue(":id_game", cmd.game_id());
	query.bindValue(":do_not_hide", cmd.do_not_hide());
	
	if (!sqlInterface->execSqlQuery(query))
		return Response::RespInternalError;
	return query.numRowsAffected() > 0 ? Response::RespOk : Response::RespNameNotFound;
}

Response::ResponseCode ServerSocketInterface::cmdReplayDeleteMatch(const Command_ReplayDeleteMatch &cmd, ResponseContainer & /*rc*/)
{
	if (authState != PasswordRight)
		return Response::RespFunctionNotAllowed;
	
	if (!sqlInterface->checkSql())
		return Response::RespInternalError;
	
	QSqlQuery query(sqlInterface->getDatabase());
	query.prepare("delete from " + servatrice->getDbPrefix() + "_replays_access where id_player = :id_player and id_game = :id_game");
	query.bindValue(":id_player", userInfo->id());
	query.bindValue(":id_game", cmd.game_id());
	
	if (!sqlInterface->execSqlQuery(query))
		return Response::RespInternalError;
	return query.numRowsAffected() > 0 ? Response::RespOk : Response::RespNameNotFound;
}


// MODERATOR FUNCTIONS.
// May be called by admins and moderators. Permission is checked by the calling function.

Response::ResponseCode ServerSocketInterface::cmdBanFromServer(const Command_BanFromServer &cmd, ResponseContainer & /*rc*/)
{
	if (!sqlInterface->checkSql())
		return Response::RespInternalError;
	
	QString userName = QString::fromStdString(cmd.user_name());
	QString address = QString::fromStdString(cmd.address());
	int minutes = cmd.minutes();
	
	QSqlQuery query(sqlInterface->getDatabase());
	query.prepare("insert into " + servatrice->getDbPrefix() + "_bans (user_name, ip_address, id_admin, time_from, minutes, reason, visible_reason) values(:user_name, :ip_address, :id_admin, NOW(), :minutes, :reason, :visible_reason)");
	query.bindValue(":user_name", userName);
	query.bindValue(":ip_address", address);
	query.bindValue(":id_admin", userInfo->id());
	query.bindValue(":minutes", minutes);
	query.bindValue(":reason", QString::fromStdString(cmd.reason()));
	query.bindValue(":visible_reason", QString::fromStdString(cmd.visible_reason()));
	sqlInterface->execSqlQuery(query);
	
	servatrice->clientsLock.lockForRead();
	QList<ServerSocketInterface *> userList = servatrice->getUsersWithAddressAsList(QHostAddress(address));
	ServerSocketInterface *user = static_cast<ServerSocketInterface *>(server->getUsers().value(userName));
	if (user && !userList.contains(user))
		userList.append(user);
	if (!userList.isEmpty()) {
		Event_ConnectionClosed event;
		event.set_reason(Event_ConnectionClosed::BANNED);
		if (cmd.has_visible_reason())
			event.set_reason_str(cmd.visible_reason());
		if (minutes)
			event.set_end_time(QDateTime::currentDateTime().addSecs(60 * minutes).toTime_t());
		for (int i = 0; i < userList.size(); ++i) {
			SessionEvent *se = userList[i]->prepareSessionEvent(event);
			userList[i]->sendProtocolItem(*se);
			delete se;
			QMetaObject::invokeMethod(userList[i], "prepareDestroy", Qt::QueuedConnection);
		}
	}
	servatrice->clientsLock.unlock();
	
	return Response::RespOk;
}

// ADMIN FUNCTIONS.
// Permission is checked by the calling function.

Response::ResponseCode ServerSocketInterface::cmdUpdateServerMessage(const Command_UpdateServerMessage & /*cmd*/, ResponseContainer & /*rc*/)
{
	QMetaObject::invokeMethod(server, "updateLoginMessage");
	return Response::RespOk;
}

Response::ResponseCode ServerSocketInterface::cmdShutdownServer(const Command_ShutdownServer &cmd, ResponseContainer & /*rc*/)
{
	QMetaObject::invokeMethod(server, "scheduleShutdown", Q_ARG(QString, QString::fromStdString(cmd.reason())), Q_ARG(int, cmd.minutes()));
	return Response::RespOk;
}
