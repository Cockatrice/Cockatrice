#include <QDebug>
#include "server_protocolhandler.h"
#include "server_room.h"
#include "server_card.h"
#include "server_arrow.h"
#include "server_cardzone.h"
#include "server_counter.h"
#include "server_game.h"
#include "server_player.h"
#include "decklist.h"
#include "get_pb_extension.h"
#include <QDateTime>
#include "pb/serverinfo_zone.pb.h"
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
#include "pb/response.pb.h"
#include "pb/response_login.pb.h"
#include "pb/response_list_users.pb.h"
#include "pb/response_get_games_of_user.pb.h"
#include "pb/response_get_user_info.pb.h"
#include "pb/response_join_room.pb.h"
#include "pb/event_list_rooms.pb.h"
#include "pb/event_server_message.pb.h"
#include "pb/event_user_message.pb.h"
#include "pb/event_game_joined.pb.h"
#include "pb/event_game_state_changed.pb.h"
#include "pb/event_room_say.pb.h"
#include <google/protobuf/descriptor.h>

Server_ProtocolHandler::Server_ProtocolHandler(Server *_server, QObject *parent)
	: QObject(parent), Server_AbstractUserInterface(_server), authState(NotLoggedIn), acceptsUserListChanges(false), acceptsRoomListChanges(false), sessionId(-1), timeRunning(0), lastDataReceived(0)
{
	connect(server, SIGNAL(pingClockTimeout()), this, SLOT(pingClockTimeout()));
}

Server_ProtocolHandler::~Server_ProtocolHandler()
{
}

void Server_ProtocolHandler::prepareDestroy()
{
	qDebug("Server_ProtocolHandler::prepareDestroy");
	
	server->removeClient(this);
	
	QMapIterator<int, Server_Room *> roomIterator(rooms);
	while (roomIterator.hasNext())
		roomIterator.next().value()->removeClient(this);
	
	gameListMutex.lock();
	QMap<int, QPair<int, int> > tempGames(games);
	gameListMutex.unlock();
	
	server->roomsLock.lockForRead();
	QMapIterator<int, QPair<int, int> > gameIterator(tempGames);
	while (gameIterator.hasNext()) {
		gameIterator.next();
		
		Server_Room *r = server->getRooms().value(gameIterator.value().first);
		if (!r)
			continue;
		r->gamesMutex.lock();
		Server_Game *g = r->getGames().value(gameIterator.key());
		if (!g) {
			r->gamesMutex.unlock();
			continue;
		}
		g->gameMutex.lock();
		Server_Player *p = g->getPlayer(gameIterator.value().second);
		if (!p) {
			g->gameMutex.unlock();
			r->gamesMutex.unlock();
			continue;
		}
		
		if ((authState == UnknownUser) || p->getSpectator())
			g->removePlayer(p);
		else
			p->setProtocolHandler(0);
		
		g->gameMutex.unlock();
		r->gamesMutex.unlock();
	}
	server->roomsLock.unlock();
	
	deleteLater();
}

void Server_ProtocolHandler::playerRemovedFromGame(Server_Game *game)
{
	qDebug() << "Server_ProtocolHandler::playerRemovedFromGame(): gameId =" << game->getGameId();
	
	QMutexLocker locker(&gameListMutex);
	games.remove(game->getGameId());
}

void Server_ProtocolHandler::sendProtocolItem(const Response &item)
{
	ServerMessage msg;
	msg.mutable_response()->CopyFrom(item);
	msg.set_message_type(ServerMessage::RESPONSE);
	
	transmitProtocolItem(msg);
}

void Server_ProtocolHandler::sendProtocolItem(const SessionEvent &item)
{
	ServerMessage msg;
	msg.mutable_session_event()->CopyFrom(item);
	msg.set_message_type(ServerMessage::SESSION_EVENT);
	
	transmitProtocolItem(msg);
}

void Server_ProtocolHandler::sendProtocolItem(const GameEventContainer &item)
{
	ServerMessage msg;
	msg.mutable_game_event_container()->CopyFrom(item);
	msg.set_message_type(ServerMessage::GAME_EVENT_CONTAINER);
	
	transmitProtocolItem(msg);
}

void Server_ProtocolHandler::sendProtocolItem(const RoomEvent &item)
{
	ServerMessage msg;
	msg.mutable_room_event()->CopyFrom(item);
	msg.set_message_type(ServerMessage::ROOM_EVENT);
	
	transmitProtocolItem(msg);
}

Response::ResponseCode Server_ProtocolHandler::processSessionCommandContainer(const CommandContainer &cont, ResponseContainer &rc)
{
	Response::ResponseCode finalResponseCode = Response::RespOk;
	for (int i = cont.session_command_size() - 1; i >= 0; --i) {
		Response::ResponseCode resp = Response::RespInvalidCommand;
		const SessionCommand &sc = cont.session_command(i);
		const int num = getPbExtension(sc);
		if (num != SessionCommand::PING)
			emit logDebugMessage(QString::fromStdString(sc.ShortDebugString()), this);
		switch ((SessionCommand::SessionCommandType) num) {
			case SessionCommand::PING: resp = cmdPing(sc.GetExtension(Command_Ping::ext), rc); break;
			case SessionCommand::LOGIN: resp = cmdLogin(sc.GetExtension(Command_Login::ext), rc); break;
			case SessionCommand::MESSAGE: resp = cmdMessage(sc.GetExtension(Command_Message::ext), rc); break;
			case SessionCommand::ADD_TO_LIST: resp = cmdAddToList(sc.GetExtension(Command_AddToList::ext), rc); break;
			case SessionCommand::REMOVE_FROM_LIST: resp = cmdRemoveFromList(sc.GetExtension(Command_RemoveFromList::ext), rc); break;
			case SessionCommand::DECK_LIST: resp = cmdDeckList(sc.GetExtension(Command_DeckList::ext), rc); break;
			case SessionCommand::DECK_NEW_DIR: resp = cmdDeckNewDir(sc.GetExtension(Command_DeckNewDir::ext), rc); break;
			case SessionCommand::DECK_DEL_DIR: resp = cmdDeckDelDir(sc.GetExtension(Command_DeckDelDir::ext), rc); break;
			case SessionCommand::DECK_DEL: resp = cmdDeckDel(sc.GetExtension(Command_DeckDel::ext), rc); break;
			case SessionCommand::DECK_UPLOAD: resp = cmdDeckUpload(sc.GetExtension(Command_DeckUpload::ext), rc); break;
			case SessionCommand::DECK_DOWNLOAD: resp = cmdDeckDownload(sc.GetExtension(Command_DeckDownload::ext), rc); break;
			case SessionCommand::REPLAY_LIST: resp = cmdReplayList(sc.GetExtension(Command_ReplayList::ext), rc); break;
			case SessionCommand::REPLAY_DOWNLOAD: resp = cmdReplayDownload(sc.GetExtension(Command_ReplayDownload::ext), rc); break;
			case SessionCommand::REPLAY_MODIFY_MATCH: resp = cmdReplayModifyMatch(sc.GetExtension(Command_ReplayModifyMatch::ext), rc); break;
			case SessionCommand::GET_GAMES_OF_USER: resp = cmdGetGamesOfUser(sc.GetExtension(Command_GetGamesOfUser::ext), rc); break;
			case SessionCommand::GET_USER_INFO: resp = cmdGetUserInfo(sc.GetExtension(Command_GetUserInfo::ext), rc); break;
			case SessionCommand::LIST_ROOMS: resp = cmdListRooms(sc.GetExtension(Command_ListRooms::ext), rc); break;
			case SessionCommand::JOIN_ROOM: resp = cmdJoinRoom(sc.GetExtension(Command_JoinRoom::ext), rc); break;
			case SessionCommand::LIST_USERS: resp = cmdListUsers(sc.GetExtension(Command_ListUsers::ext), rc); break;
		}
		if ((resp != Response::RespOk) && (resp != Response::RespNothing))
			finalResponseCode = resp;
	}
	return finalResponseCode;
}

Response::ResponseCode Server_ProtocolHandler::processRoomCommandContainer(const CommandContainer &cont, ResponseContainer &rc)
{
	if (authState == NotLoggedIn)
		return Response::RespLoginNeeded;

	QReadLocker locker(&server->roomsLock);
	Server_Room *room = rooms.value(cont.room_id(), 0);
	if (!room)
		return Response::RespNotInRoom;
	
	Response::ResponseCode finalResponseCode = Response::RespOk;
	for (int i = cont.room_command_size() - 1; i >= 0; --i) {
		Response::ResponseCode resp = Response::RespInvalidCommand;
		const RoomCommand &sc = cont.room_command(i);
		const int num = getPbExtension(sc);
		emit logDebugMessage(QString::fromStdString(sc.ShortDebugString()), this);
		switch ((RoomCommand::RoomCommandType) num) {
			case RoomCommand::LEAVE_ROOM: resp = cmdLeaveRoom(sc.GetExtension(Command_LeaveRoom::ext), room, rc); break;
			case RoomCommand::ROOM_SAY: resp = cmdRoomSay(sc.GetExtension(Command_RoomSay::ext), room, rc); break;
			case RoomCommand::CREATE_GAME: resp = cmdCreateGame(sc.GetExtension(Command_CreateGame::ext), room, rc); break;
			case RoomCommand::JOIN_GAME: resp = cmdJoinGame(sc.GetExtension(Command_JoinGame::ext), room, rc); break;
		}
		if ((resp != Response::RespOk) && (resp != Response::RespNothing))
			finalResponseCode = resp;
	}
	return finalResponseCode;
}

Response::ResponseCode Server_ProtocolHandler::processGameCommandContainer(const CommandContainer &cont, ResponseContainer &rc)
{
	if (authState == NotLoggedIn)
		return Response::RespLoginNeeded;
	
	gameListMutex.lock();
	if (!games.contains(cont.game_id())) {
		gameListMutex.unlock();
		return Response::RespNotInRoom;
	}
	const QPair<int, int> roomIdAndPlayerId = games.value(cont.game_id());
	gameListMutex.unlock();
	
	server->roomsLock.lockForRead();
	Server_Room *room = server->getRooms().value(roomIdAndPlayerId.first);
	if (!room) {
		server->roomsLock.unlock();
		return Response::RespNotInRoom;
	}
	room->gamesMutex.lock();
	Server_Game *game = room->getGames().value(cont.game_id());
	if (!game) {
		room->gamesMutex.unlock();
		server->roomsLock.unlock();
		return Response::RespNotInRoom;
	}
	game->gameMutex.lock();
	Server_Player *player = game->getPlayer(roomIdAndPlayerId.second);
	if (!player) {
		game->gameMutex.unlock();
		room->gamesMutex.unlock();
		server->roomsLock.unlock();
		return Response::RespNotInRoom;
	}
	
	GameEventStorage ges;
	Response::ResponseCode finalResponseCode = Response::RespOk;
	for (int i = cont.game_command_size() - 1; i >= 0; --i) {
		const GameCommand &sc = cont.game_command(i);
		emit logDebugMessage(QString::fromStdString(sc.ShortDebugString()), this);
		
		Response::ResponseCode resp = player->processGameCommand(sc, rc, ges);

		if ((resp != Response::RespOk) && (resp != Response::RespNothing))
			finalResponseCode = resp;
	}
	ges.sendToGame(game);
	
	game->gameMutex.unlock();
	room->gamesMutex.unlock();
	server->roomsLock.unlock();
	
	return finalResponseCode;
}

Response::ResponseCode Server_ProtocolHandler::processModeratorCommandContainer(const CommandContainer &cont, ResponseContainer &rc)
{
	if (!userInfo)
		return Response::RespLoginNeeded;
	if (!(userInfo->user_level() & ServerInfo_User::IsModerator))
		return Response::RespLoginNeeded;

	Response::ResponseCode finalResponseCode = Response::RespOk;
	for (int i = cont.moderator_command_size() - 1; i >= 0; --i) {
		Response::ResponseCode resp = Response::RespInvalidCommand;
		const ModeratorCommand &sc = cont.moderator_command(i);
		const int num = getPbExtension(sc);
		emit logDebugMessage(QString::fromStdString(sc.ShortDebugString()), this);
		switch ((ModeratorCommand::ModeratorCommandType) num) {
			case ModeratorCommand::BAN_FROM_SERVER: resp = cmdBanFromServer(sc.GetExtension(Command_BanFromServer::ext), rc); break;
		}
		if ((resp != Response::RespOk) && (resp != Response::RespNothing))
			finalResponseCode = resp;
	}
	return finalResponseCode;
}

Response::ResponseCode Server_ProtocolHandler::processAdminCommandContainer(const CommandContainer &cont, ResponseContainer &rc)
{
	if (!userInfo)
		return Response::RespLoginNeeded;
	if (!(userInfo->user_level() & ServerInfo_User::IsAdmin))
		return Response::RespLoginNeeded;

	Response::ResponseCode finalResponseCode = Response::RespOk;
	for (int i = cont.admin_command_size() - 1; i >= 0; --i) {
		Response::ResponseCode resp = Response::RespInvalidCommand;
		const AdminCommand &sc = cont.admin_command(i);
		const int num = getPbExtension(sc);
		emit logDebugMessage(QString::fromStdString(sc.ShortDebugString()), this);
		switch ((AdminCommand::AdminCommandType) num) {
			case AdminCommand::SHUTDOWN_SERVER: resp = cmdShutdownServer(sc.GetExtension(Command_ShutdownServer::ext), rc); break;
			case AdminCommand::UPDATE_SERVER_MESSAGE: resp = cmdUpdateServerMessage(sc.GetExtension(Command_UpdateServerMessage::ext), rc); break;
		}
		if ((resp != Response::RespOk) && (resp != Response::RespNothing))
			finalResponseCode = resp;
	}
	return finalResponseCode;
}

void Server_ProtocolHandler::processCommandContainer(const CommandContainer &cont)
{
	lastDataReceived = timeRunning;
	
	ResponseContainer responseContainer;
	Response::ResponseCode finalResponseCode;
	
	if (cont.game_command_size())
		finalResponseCode = processGameCommandContainer(cont, responseContainer);
	else if (cont.room_command_size())
		finalResponseCode = processRoomCommandContainer(cont, responseContainer);
	else if (cont.session_command_size())
		finalResponseCode = processSessionCommandContainer(cont, responseContainer);
	else if (cont.moderator_command_size())
		finalResponseCode = processModeratorCommandContainer(cont, responseContainer);
	else if (cont.admin_command_size())
		finalResponseCode = processAdminCommandContainer(cont, responseContainer);
	else
		finalResponseCode = Response::RespInvalidCommand;
	
	const QList<QPair<ServerMessage::MessageType, ::google::protobuf::Message *> > &preResponseQueue = responseContainer.getPreResponseQueue();
	for (int i = 0; i < preResponseQueue.size(); ++i)
		sendProtocolItemByType(preResponseQueue[i].first, *preResponseQueue[i].second);
	
	Response response;
	response.set_cmd_id(cont.cmd_id());
	response.set_response_code(finalResponseCode);
	::google::protobuf::Message *responseExtension = responseContainer.getResponseExtension();
	if (responseExtension)
		response.GetReflection()->MutableMessage(&response, responseExtension->GetDescriptor()->FindExtensionByName("ext"))->CopyFrom(*responseExtension);
	sendProtocolItem(response);
	
	const QList<QPair<ServerMessage::MessageType, ::google::protobuf::Message *> > &postResponseQueue = responseContainer.getPostResponseQueue();
	for (int i = 0; i < postResponseQueue.size(); ++i)
		sendProtocolItemByType(postResponseQueue[i].first, *postResponseQueue[i].second);
}

void Server_ProtocolHandler::pingClockTimeout()
{
	int interval = server->getMessageCountingInterval();
	if (interval > 0) {
		messageSizeOverTime.prepend(0);
		if (messageSizeOverTime.size() > server->getMessageCountingInterval())
			messageSizeOverTime.removeLast();
		messageCountOverTime.prepend(0);
		if (messageCountOverTime.size() > server->getMessageCountingInterval())
			messageCountOverTime.removeLast();
	}
	
	if (timeRunning - lastDataReceived > server->getMaxPlayerInactivityTime())
		prepareDestroy();
	++timeRunning;
}

Response::ResponseCode Server_ProtocolHandler::cmdPing(const Command_Ping & /*cmd*/, ResponseContainer & /*rc*/)
{
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdLogin(const Command_Login &cmd, ResponseContainer &rc)
{
	QString userName = QString::fromStdString(cmd.user_name()).simplified();
	if (userName.isEmpty() || (userInfo != 0))
		return Response::RespContextError;
	QString reasonStr;
	AuthenticationResult res = server->loginUser(this, userName, QString::fromStdString(cmd.password()), reasonStr);
	switch (res) {
		case UserIsBanned: {
			Response_Login *re = new Response_Login;
			re->set_denied_reason_str(reasonStr.toStdString());
			rc.setResponseExtension(re);
			return Response::RespUserIsBanned;
		}
		case NotLoggedIn: return Response::RespWrongPassword;
		case WouldOverwriteOldSession: return Response::RespWouldOverwriteOldSession;
		default: authState = res;
	}
	
	userName = QString::fromStdString(userInfo->name());
	Event_ServerMessage event;
	event.set_message(server->getLoginMessage().toStdString());
	rc.enqueuePostResponseItem(ServerMessage::SESSION_EVENT, prepareSessionEvent(event));
	
	Response_Login *re = new Response_Login;
	re->mutable_user_info()->CopyFrom(copyUserInfo(true));
	
	if (authState == PasswordRight) {
		QMapIterator<QString, ServerInfo_User> buddyIterator(server->getBuddyList(userName));
		while (buddyIterator.hasNext())
			re->add_buddy_list()->CopyFrom(buddyIterator.next().value());
	
		QMapIterator<QString, ServerInfo_User> ignoreIterator(server->getIgnoreList(userName));
		while (ignoreIterator.hasNext())
			re->add_ignore_list()->CopyFrom(ignoreIterator.next().value());
	}
	
	server->roomsLock.lockForRead();
	
	QMapIterator<int, Server_Room *> roomIterator(server->getRooms());
	while (roomIterator.hasNext()) {
		Server_Room *room = roomIterator.next().value();
		room->gamesMutex.lock();
		QMapIterator<int, Server_Game *> gameIterator(room->getGames());
		while (gameIterator.hasNext()) {
			Server_Game *game = gameIterator.next().value();
			QMutexLocker gameLocker(&game->gameMutex);
			const QList<Server_Player *> &gamePlayers = game->getPlayers().values();
			for (int j = 0; j < gamePlayers.size(); ++j)
				if (gamePlayers[j]->getUserInfo()->name() == userInfo->name()) {
					gamePlayers[j]->setProtocolHandler(this);
					
					gameListMutex.lock();
					games.insert(game->getGameId(), QPair<int, int>(room->getId(), gamePlayers[j]->getPlayerId()));
					gameListMutex.unlock();
					
					Event_GameJoined event1;
					event1.set_game_id(game->getGameId());
					event1.set_game_description(game->getDescription().toStdString());
					event1.set_host_id(game->getHostId());
					event1.set_player_id(gamePlayers[j]->getPlayerId());
					event1.set_spectator(gamePlayers[j]->getSpectator());
					event1.set_spectators_can_talk(game->getSpectatorsCanTalk());
					event1.set_spectators_see_everything(game->getSpectatorsSeeEverything());
					event1.set_resuming(true);
					rc.enqueuePostResponseItem(ServerMessage::SESSION_EVENT, prepareSessionEvent(event1));
					
					Event_GameStateChanged event2;
					QListIterator<ServerInfo_Player> gameStateIterator(game->getGameState(gamePlayers[j], gamePlayers[j]->getSpectator() && game->getSpectatorsSeeEverything(), true));
					while (gameStateIterator.hasNext())
						event2.add_player_list()->CopyFrom(gameStateIterator.next());
					event2.set_seconds_elapsed(game->getSecondsElapsed());
					event2.set_game_started(game->getGameStarted());
					event2.set_active_player_id(game->getActivePlayer());
					event2.set_active_phase(game->getActivePhase());
					rc.enqueuePostResponseItem(ServerMessage::GAME_EVENT_CONTAINER, game->prepareGameEvent(event2, -1));
					
					break;
				}
		}
		room->gamesMutex.unlock();
	}
	server->roomsLock.unlock();
	
	rc.setResponseExtension(re);
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdMessage(const Command_Message &cmd, ResponseContainer &rc)
{
	if (authState == NotLoggedIn)
		return Response::RespLoginNeeded;
	
	QReadLocker locker(&server->clientsLock);
	
	QString receiver = QString::fromStdString(cmd.user_name());
	Server_AbstractUserInterface *userInterface = server->getUsers().value(receiver);
	if (!userInterface) {
		userInterface = server->getExternalUsers().value(receiver);
		if (!userInterface)
			return Response::RespNameNotFound;
	}
	if (server->isInIgnoreList(receiver, QString::fromStdString(userInfo->name())))
		return Response::RespInIgnoreList;
	
	Event_UserMessage event;
	event.set_sender_name(userInfo->name());
	event.set_receiver_name(cmd.user_name());
	event.set_message(cmd.message());
	
	SessionEvent *se = prepareSessionEvent(event);
	userInterface->sendProtocolItem(*se);
	rc.enqueuePreResponseItem(ServerMessage::SESSION_EVENT, se);
	
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdGetGamesOfUser(const Command_GetGamesOfUser &cmd, ResponseContainer &rc)
{
	if (authState == NotLoggedIn)
		return Response::RespLoginNeeded;
	
	server->clientsLock.lockForRead();
	if (!server->getUsers().contains(QString::fromStdString(cmd.user_name())))
		return Response::RespNameNotFound;
	server->clientsLock.unlock();
	
	Response_GetGamesOfUser *re = new Response_GetGamesOfUser;
	server->roomsLock.lockForRead();
	QMapIterator<int, Server_Room *> roomIterator(server->getRooms());
	while (roomIterator.hasNext()) {
		Server_Room *room = roomIterator.next().value();
		room->gamesMutex.lock();
		re->add_room_list()->CopyFrom(room->getInfo(false, true));
		QListIterator<ServerInfo_Game> gameIterator(room->getGamesOfUser(QString::fromStdString(cmd.user_name())));
		while (gameIterator.hasNext())
			re->add_game_list()->CopyFrom(gameIterator.next());
		room->gamesMutex.unlock();
	}
	server->roomsLock.unlock();
	
	rc.setResponseExtension(re);
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdGetUserInfo(const Command_GetUserInfo &cmd, ResponseContainer &rc)
{
	if (authState == NotLoggedIn)
		return Response::RespLoginNeeded;
	
	QString userName = QString::fromStdString(cmd.user_name());
	Response_GetUserInfo *re = new Response_GetUserInfo;
	if (userName.isEmpty())
		re->mutable_user_info()->CopyFrom(*userInfo);
	else {
		
		QReadLocker locker(&server->clientsLock);
		
		ServerInfo_User_Container *infoSource;
		if (server->getUsers().contains(userName))
			infoSource = server->getUsers().value(userName);
		else if (server->getExternalUsers().contains(userName))
			infoSource = server->getExternalUsers().value(userName);
		else
			return Response::RespNameNotFound;
		
		re->mutable_user_info()->CopyFrom(infoSource->copyUserInfo(true, userInfo->user_level() & ServerInfo_User::IsModerator));
	}
	
	rc.setResponseExtension(re);
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdListRooms(const Command_ListRooms & /*cmd*/, ResponseContainer &rc)
{
	if (authState == NotLoggedIn)
		return Response::RespLoginNeeded;
	
	Event_ListRooms event;
	QMapIterator<int, Server_Room *> roomIterator(server->getRooms());
	while (roomIterator.hasNext())
		event.add_room_list()->CopyFrom(roomIterator.next().value()->getInfo(false));
	rc.enqueuePreResponseItem(ServerMessage::SESSION_EVENT, prepareSessionEvent(event));
	
	acceptsRoomListChanges = true;
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdJoinRoom(const Command_JoinRoom &cmd, ResponseContainer &rc)
{
	if (authState == NotLoggedIn)
		return Response::RespLoginNeeded;
	
	if (rooms.contains(cmd.room_id()))
		return Response::RespContextError;
	
	QReadLocker serverLocker(&server->roomsLock);
	Server_Room *r = server->getRooms().value(cmd.room_id(), 0);
	if (!r)
		return Response::RespNameNotFound;
	
	r->addClient(this);
	rooms.insert(r->getId(), r);
	
	Event_RoomSay joinMessageEvent;
	joinMessageEvent.set_message(r->getJoinMessage().toStdString());
	rc.enqueuePostResponseItem(ServerMessage::ROOM_EVENT, r->prepareRoomEvent(joinMessageEvent));
	
	Response_JoinRoom *re = new Response_JoinRoom;
	re->mutable_room_info()->CopyFrom(r->getInfo(true));
	
	rc.setResponseExtension(re);
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdListUsers(const Command_ListUsers & /*cmd*/, ResponseContainer &rc)
{
	if (authState == NotLoggedIn)
		return Response::RespLoginNeeded;
	
	Response_ListUsers *re = new Response_ListUsers;
	server->clientsLock.lockForRead();
	QMapIterator<QString, Server_ProtocolHandler *> userIterator = server->getUsers();
	while (userIterator.hasNext())
		re->add_user_list()->CopyFrom(userIterator.next().value()->copyUserInfo(false));
	QMapIterator<QString, Server_AbstractUserInterface *> extIterator = server->getExternalUsers();
	while (extIterator.hasNext())
		re->add_user_list()->CopyFrom(extIterator.next().value()->copyUserInfo(false));
	server->clientsLock.unlock();
	
	acceptsUserListChanges = true;
	
	rc.setResponseExtension(re);
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdLeaveRoom(const Command_LeaveRoom & /*cmd*/, Server_Room *room, ResponseContainer & /*rc*/)
{
	rooms.remove(room->getId());
	room->removeClient(this);
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdRoomSay(const Command_RoomSay &cmd, Server_Room *room, ResponseContainer & /*rc*/)
{
	QString msg = QString::fromStdString(cmd.message());
	
	if (server->getMessageCountingInterval() > 0) {
		int totalSize = 0, totalCount = 0;
		if (messageSizeOverTime.isEmpty())
			messageSizeOverTime.prepend(0);
		messageSizeOverTime[0] += msg.size();
		for (int i = 0; i < messageSizeOverTime.size(); ++i)
			totalSize += messageSizeOverTime[i];
		
		if (messageCountOverTime.isEmpty())
			messageCountOverTime.prepend(0);
		++messageCountOverTime[0];
		for (int i = 0; i < messageCountOverTime.size(); ++i)
			totalCount += messageCountOverTime[i];
		
		if ((totalSize > server->getMaxMessageSizePerInterval()) || (totalCount > server->getMaxMessageCountPerInterval()))
			return Response::RespChatFlood;
	}
	msg.replace(QChar('\n'), QChar(' '));
	
	room->say(QString::fromStdString(userInfo->name()), msg);
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdCreateGame(const Command_CreateGame &cmd, Server_Room *room, ResponseContainer &rc)
{
	if (authState == NotLoggedIn)
		return Response::RespLoginNeeded;
	
	QMutexLocker roomLocker(&room->gamesMutex);
	
	if (server->getMaxGamesPerUser() > 0)
		if (room->getGamesCreatedByUser(QString::fromStdString(userInfo->name())) >= server->getMaxGamesPerUser())
			return Response::RespContextError;
	
	QList<int> gameTypes;
	for (int i = cmd.game_type_ids_size() - 1; i >= 0; --i)
		gameTypes.append(cmd.game_type_ids(i));
	
	QString description = QString::fromStdString(cmd.description());
	if (description.size() > 60)
		description = description.left(60);
	
	Server_Game *game = new Server_Game(this, server->getNextGameId(), description, QString::fromStdString(cmd.password()), cmd.max_players(), gameTypes, cmd.only_buddies(), cmd.only_registered(), cmd.spectators_allowed(), cmd.spectators_need_password(), cmd.spectators_can_talk(), cmd.spectators_see_everything(), room);
	game->moveToThread(room->thread());

	game->gameMutex.lock();
	room->addGame(game);
	
	Server_Player *creator = game->getPlayers().values().first();
	
	gameListMutex.lock();
	games.insert(game->getGameId(), QPair<int, int>(room->getId(), creator->getPlayerId()));
	gameListMutex.unlock();
	
	Event_GameJoined event1;
	event1.set_game_id(game->getGameId());
	event1.set_game_description(game->getDescription().toStdString());
	event1.set_host_id(creator->getPlayerId());
	event1.set_player_id(creator->getPlayerId());
	event1.set_spectator(false);
	event1.set_spectators_can_talk(game->getSpectatorsCanTalk());
	event1.set_spectators_see_everything(game->getSpectatorsSeeEverything());
	event1.set_resuming(false);
	rc.enqueuePreResponseItem(ServerMessage::SESSION_EVENT, prepareSessionEvent(event1));
	
	Event_GameStateChanged event2;
	QListIterator<ServerInfo_Player> gameStateIterator(game->getGameState(creator, false, true));
	while (gameStateIterator.hasNext())
		event2.add_player_list()->CopyFrom(gameStateIterator.next());
	event2.set_seconds_elapsed(0);
	event2.set_game_started(game->getGameStarted());
	event2.set_active_player_id(game->getActivePlayer());
	event2.set_active_phase(game->getActivePhase());
	rc.enqueuePreResponseItem(ServerMessage::GAME_EVENT_CONTAINER, game->prepareGameEvent(event2, -1));

	game->gameMutex.unlock();
	
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdJoinGame(const Command_JoinGame &cmd, Server_Room *room, ResponseContainer &rc)
{
	if (authState == NotLoggedIn)
		return Response::RespLoginNeeded;
	
	room->gamesMutex.lock();
	Server_Game *g = room->getGames().value(cmd.game_id());
	if (!g) {
		room->gamesMutex.unlock();
		return Response::RespNameNotFound;
	}
	
	g->gameMutex.lock();
	
	Response::ResponseCode result = g->checkJoin(userInfo, QString::fromStdString(cmd.password()), cmd.spectator(), cmd.override_restrictions());
	if (result == Response::RespOk) {
		Server_Player *player = g->addPlayer(this, cmd.spectator());
		
		gameListMutex.lock();
		games.insert(cmd.game_id(), QPair<int, int>(room->getId(), player->getPlayerId()));
		gameListMutex.unlock();
		
		Event_GameJoined event1;
		event1.set_game_id(g->getGameId());
		event1.set_game_description(g->getDescription().toStdString());
		event1.set_host_id(g->getHostId());
		event1.set_player_id(player->getPlayerId());
		event1.set_spectator(cmd.spectator());
		event1.set_spectators_can_talk(g->getSpectatorsCanTalk());
		event1.set_spectators_see_everything(g->getSpectatorsSeeEverything());
		event1.set_resuming(false);
		rc.enqueuePostResponseItem(ServerMessage::SESSION_EVENT, prepareSessionEvent(event1));
		
		Event_GameStateChanged event2;
		QListIterator<ServerInfo_Player> gameStateIterator(g->getGameState(player, false, true));
		while (gameStateIterator.hasNext())
			event2.add_player_list()->CopyFrom(gameStateIterator.next());
		event2.set_seconds_elapsed(g->getSecondsElapsed());
		event2.set_game_started(g->getGameStarted());
		event2.set_active_player_id(g->getActivePlayer());
		event2.set_active_phase(g->getActivePhase());
		rc.enqueuePostResponseItem(ServerMessage::GAME_EVENT_CONTAINER, g->prepareGameEvent(event2, -1));
	}
	
	g->gameMutex.unlock();
	room->gamesMutex.unlock();
	
	return result;
}
