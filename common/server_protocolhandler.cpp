#include <QDebug>
#include "rng_abstract.h"
#include "server_protocolhandler.h"
#include "server_room.h"
#include "server_card.h"
#include "server_arrow.h"
#include "server_cardzone.h"
#include "server_counter.h"
#include "server_game.h"
#include "server_player.h"
#include "decklist.h"
#include <QDateTime>
#include "pb/serverinfo_zone.pb.h"
#include "pb/commands.pb.h"
#include "pb/command_attach_card.pb.h"
#include "pb/command_concede.pb.h"
#include "pb/command_create_arrow.pb.h"
#include "pb/command_create_counter.pb.h"
#include "pb/command_create_token.pb.h"
#include "pb/command_deck_select.pb.h"
#include "pb/command_del_counter.pb.h"
#include "pb/command_delete_arrow.pb.h"
#include "pb/command_draw_cards.pb.h"
#include "pb/command_dump_zone.pb.h"
#include "pb/command_flip_card.pb.h"
#include "pb/command_game_say.pb.h"
#include "pb/command_inc_card_counter.pb.h"
#include "pb/command_inc_counter.pb.h"
#include "pb/command_kick_from_game.pb.h"
#include "pb/command_leave_game.pb.h"
#include "pb/command_move_card.pb.h"
#include "pb/command_mulligan.pb.h"
#include "pb/command_next_turn.pb.h"
#include "pb/command_ready_start.pb.h"
#include "pb/command_reveal_cards.pb.h"
#include "pb/command_roll_die.pb.h"
#include "pb/command_set_active_phase.pb.h"
#include "pb/command_set_card_attr.pb.h"
#include "pb/command_set_card_counter.pb.h"
#include "pb/command_set_counter.pb.h"
#include "pb/command_set_sideboard_plan.pb.h"
#include "pb/command_shuffle.pb.h"
#include "pb/command_stop_dump_zone.pb.h"
#include "pb/command_undo_draw.pb.h"
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
#include "pb/response_deck_download.pb.h"
#include "pb/response_dump_zone.pb.h"
#include "pb/event_list_rooms.pb.h"
#include "pb/event_server_message.pb.h"
#include "pb/event_user_message.pb.h"
#include "pb/event_game_say.pb.h"
#include "pb/event_game_joined.pb.h"
#include "pb/event_game_state_changed.pb.h"
#include "pb/event_shuffle.pb.h"
#include "pb/event_roll_die.pb.h"
#include "pb/event_player_properties_changed.pb.h"
#include "pb/event_create_arrow.pb.h"
#include "pb/event_delete_arrow.pb.h"
#include "pb/event_set_card_counter.pb.h"
#include "pb/event_flip_card.pb.h"
#include "pb/event_attach_card.pb.h"
#include "pb/event_create_token.pb.h"
#include "pb/event_room_say.pb.h"
#include "pb/event_create_counter.pb.h"
#include "pb/event_del_counter.pb.h"
#include "pb/event_set_counter.pb.h"
#include "pb/event_dump_zone.pb.h"
#include "pb/event_stop_dump_zone.pb.h"
#include "pb/event_reveal_cards.pb.h"
#include "pb/context_deck_select.pb.h"
#include "pb/context_concede.pb.h"
#include "pb/context_ready_start.pb.h"
#include "pb/context_mulligan.pb.h"
#include <google/protobuf/descriptor.h>

Server_ProtocolHandler::Server_ProtocolHandler(Server *_server, QObject *parent)
	: QObject(parent), server(_server), authState(NotLoggedIn), acceptsUserListChanges(false), acceptsRoomListChanges(false), userInfo(0), sessionId(-1), timeRunning(0), lastDataReceived(0), gameListMutex(QMutex::Recursive)
{
	connect(server, SIGNAL(pingClockTimeout()), this, SLOT(pingClockTimeout()));
}

Server_ProtocolHandler::~Server_ProtocolHandler()
{
}

// This is essentially the destructor, but it needs to be called from the
// child's destructor so that the server mutex does not get unlocked during
// finalization.
void Server_ProtocolHandler::prepareDestroy()
{
	QMutexLocker locker(&server->serverMutex);
	qDebug("Server_ProtocolHandler::prepareDestroy");
	
	server->removeClient(this);
	
	QMapIterator<int, Server_Room *> roomIterator(rooms);
	while (roomIterator.hasNext())
		roomIterator.next().value()->removeClient(this);
	
	gameListMutex.lock();
	QMapIterator<int, QPair<Server_Game *, Server_Player *> > gameIterator(games);
	while (gameIterator.hasNext()) {
		gameIterator.next();
		Server_Game *g = gameIterator.value().first;
		Server_Player *p = gameIterator.value().second;
		
		g->gameMutex.lock();
		if ((authState == UnknownUser) || p->getSpectator())
			g->removePlayer(p);
		else
			p->setProtocolHandler(0);
		g->gameMutex.unlock();
	}
	gameListMutex.unlock();

	delete userInfo;
}

void Server_ProtocolHandler::setUserInfo(const ServerInfo_User &_userInfo)
{
	userInfo = new ServerInfo_User;
	userInfo->CopyFrom(_userInfo);
}

ServerInfo_User Server_ProtocolHandler::copyUserInfo(bool complete, bool moderatorInfo) const
{
	ServerInfo_User result;
	if (userInfo) {
		result.CopyFrom(*userInfo);
		if (!moderatorInfo) {
			result.clear_address();
			result.clear_id();
		}
		if (!complete)
			result.clear_avatar_bmp();
	}
	return result;
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

void Server_ProtocolHandler::sendProtocolItem(ServerMessage::MessageType type, const ::google::protobuf::Message &item)
{
	switch (type) {
		case ServerMessage::RESPONSE: sendProtocolItem(static_cast<const Response &>(item)); break;
		case ServerMessage::SESSION_EVENT: sendProtocolItem(static_cast<const SessionEvent &>(item)); break;
		case ServerMessage::GAME_EVENT_CONTAINER: sendProtocolItem(static_cast<const GameEventContainer &>(item)); break;
		case ServerMessage::ROOM_EVENT: sendProtocolItem(static_cast<const RoomEvent &>(item)); break;
	}
}

SessionEvent *Server_ProtocolHandler::prepareSessionEvent(const ::google::protobuf::Message &sessionEvent)
{
	SessionEvent *event = new SessionEvent;
	event->GetReflection()->MutableMessage(event, sessionEvent.GetDescriptor()->FindExtensionByName("ext"))->CopyFrom(sessionEvent);
	return event;
}

Response::ResponseCode Server_ProtocolHandler::processSessionCommandContainer(const CommandContainer &cont, ResponseContainer &rc)
{
	Response::ResponseCode finalResponseCode = Response::RespOk;
	for (int i = cont.session_command_size() - 1; i >= 0; --i) {
		Response::ResponseCode resp = Response::RespInvalidCommand;
		const SessionCommand &sc = cont.session_command(i);
		std::vector< const ::google::protobuf::FieldDescriptor * > fieldList;
		sc.GetReflection()->ListFields(sc, &fieldList);
		int num = 0;
		for (unsigned int j = 0; j < fieldList.size(); ++j)
			if (fieldList[j]->is_extension()) {
				num = fieldList[j]->number();
				break;
			}
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

	Server_Room *room = rooms.value(cont.room_id(), 0);
	if (!room)
		return Response::RespNotInRoom;
	
	QMutexLocker locker(&room->roomMutex);

	Response::ResponseCode finalResponseCode = Response::RespOk;
	for (int i = cont.room_command_size() - 1; i >= 0; --i) {
		Response::ResponseCode resp = Response::RespInvalidCommand;
		const RoomCommand &sc = cont.room_command(i);
		std::vector< const ::google::protobuf::FieldDescriptor * > fieldList;
		sc.GetReflection()->ListFields(sc, &fieldList);
		int num = 0;
		for (unsigned int j = 0; j < fieldList.size(); ++j)
			if (fieldList[j]->is_extension()) {
				num = fieldList[j]->number();
				break;
			}
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
		qDebug() << "invalid game";
		return Response::RespNotInRoom;
	}
	QPair<Server_Game *, Server_Player *> gamePair = games.value(cont.game_id());
	Server_Game *game = gamePair.first;
	Server_Player *player = gamePair.second;
	
	QMutexLocker locker(&game->gameMutex);
	gameListMutex.unlock();

	GameEventStorage ges;
	Response::ResponseCode finalResponseCode = Response::RespOk;
	for (int i = cont.game_command_size() - 1; i >= 0; --i) {
		Response::ResponseCode resp = Response::RespInvalidCommand;
		const GameCommand &sc = cont.game_command(i);
		std::vector< const ::google::protobuf::FieldDescriptor * > fieldList;
		sc.GetReflection()->ListFields(sc, &fieldList);
		int num = 0;
		for (unsigned int j = 0; j < fieldList.size(); ++j)
			if (fieldList[j]->is_extension()) {
				num = fieldList[j]->number();
				break;
			}
		emit logDebugMessage(QString::fromStdString(sc.ShortDebugString()), this);
		switch ((GameCommand::GameCommandType) num) {
			case GameCommand::KICK_FROM_GAME: resp = cmdKickFromGame(sc.GetExtension(Command_KickFromGame::ext), game, player, rc, ges); break;
			case GameCommand::LEAVE_GAME: resp = cmdLeaveGame(sc.GetExtension(Command_LeaveGame::ext), game, player, rc, ges); break;
			case GameCommand::GAME_SAY: resp = cmdGameSay(sc.GetExtension(Command_GameSay::ext), game, player, rc, ges); break;
			case GameCommand::SHUFFLE: resp = cmdShuffle(sc.GetExtension(Command_Shuffle::ext), game, player, rc, ges); break;
			case GameCommand::MULLIGAN: resp = cmdMulligan(sc.GetExtension(Command_Mulligan::ext), game, player, rc, ges); break;
			case GameCommand::ROLL_DIE: resp = cmdRollDie(sc.GetExtension(Command_RollDie::ext), game, player, rc, ges); break;
			case GameCommand::DRAW_CARDS: resp = cmdDrawCards(sc.GetExtension(Command_DrawCards::ext), game, player, rc, ges); break;
			case GameCommand::UNDO_DRAW: resp = cmdUndoDraw(sc.GetExtension(Command_UndoDraw::ext), game, player, rc, ges); break;
			case GameCommand::FLIP_CARD: resp = cmdFlipCard(sc.GetExtension(Command_FlipCard::ext), game, player, rc, ges); break;
			case GameCommand::ATTACH_CARD: resp = cmdAttachCard(sc.GetExtension(Command_AttachCard::ext), game, player, rc, ges); break;
			case GameCommand::CREATE_TOKEN: resp = cmdCreateToken(sc.GetExtension(Command_CreateToken::ext), game, player, rc, ges); break;
			case GameCommand::CREATE_ARROW: resp = cmdCreateArrow(sc.GetExtension(Command_CreateArrow::ext), game, player, rc, ges); break;
			case GameCommand::DELETE_ARROW: resp = cmdDeleteArrow(sc.GetExtension(Command_DeleteArrow::ext), game, player, rc, ges); break;
			case GameCommand::SET_CARD_ATTR: resp = cmdSetCardAttr(sc.GetExtension(Command_SetCardAttr::ext), game, player, rc, ges); break;
			case GameCommand::SET_CARD_COUNTER: resp = cmdSetCardCounter(sc.GetExtension(Command_SetCardCounter::ext), game, player, rc, ges); break;
			case GameCommand::INC_CARD_COUNTER: resp = cmdIncCardCounter(sc.GetExtension(Command_IncCardCounter::ext), game, player, rc, ges); break;
			case GameCommand::READY_START: resp = cmdReadyStart(sc.GetExtension(Command_ReadyStart::ext), game, player, rc, ges); break;
			case GameCommand::CONCEDE: resp = cmdConcede(sc.GetExtension(Command_Concede::ext), game, player, rc, ges); break;
			case GameCommand::INC_COUNTER: resp = cmdIncCounter(sc.GetExtension(Command_IncCounter::ext), game, player, rc, ges); break;
			case GameCommand::CREATE_COUNTER: resp = cmdCreateCounter(sc.GetExtension(Command_CreateCounter::ext), game, player, rc, ges); break;
			case GameCommand::SET_COUNTER: resp = cmdSetCounter(sc.GetExtension(Command_SetCounter::ext), game, player, rc, ges); break;
			case GameCommand::DEL_COUNTER: resp = cmdDelCounter(sc.GetExtension(Command_DelCounter::ext), game, player, rc, ges); break;
			case GameCommand::NEXT_TURN: resp = cmdNextTurn(sc.GetExtension(Command_NextTurn::ext), game, player, rc, ges); break;
			case GameCommand::SET_ACTIVE_PHASE: resp = cmdSetActivePhase(sc.GetExtension(Command_SetActivePhase::ext), game, player, rc, ges); break;
			case GameCommand::DUMP_ZONE: resp = cmdDumpZone(sc.GetExtension(Command_DumpZone::ext), game, player, rc, ges); break;
			case GameCommand::STOP_DUMP_ZONE: resp = cmdStopDumpZone(sc.GetExtension(Command_StopDumpZone::ext), game, player, rc, ges); break;
			case GameCommand::REVEAL_CARDS: resp = cmdRevealCards(sc.GetExtension(Command_RevealCards::ext), game, player, rc, ges); break;
			case GameCommand::MOVE_CARD: resp = cmdMoveCard(sc.GetExtension(Command_MoveCard::ext), game, player, rc, ges); break;
			case GameCommand::SET_SIDEBOARD_PLAN: resp = cmdSetSideboardPlan(sc.GetExtension(Command_SetSideboardPlan::ext), game, player, rc, ges); break;
			case GameCommand::DECK_SELECT: resp = cmdDeckSelect(sc.GetExtension(Command_DeckSelect::ext), game, player, rc, ges); break;
		}
		if ((resp != Response::RespOk) && (resp != Response::RespNothing))
			finalResponseCode = resp;
	}
	ges.sendToGame(game);
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
		std::vector< const ::google::protobuf::FieldDescriptor * > fieldList;
		sc.GetReflection()->ListFields(sc, &fieldList);
		int num = 0;
		for (unsigned int j = 0; j < fieldList.size(); ++j)
			if (fieldList[j]->is_extension()) {
				num = fieldList[j]->number();
				break;
			}
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
		std::vector< const ::google::protobuf::FieldDescriptor * > fieldList;
		sc.GetReflection()->ListFields(sc, &fieldList);
		int num = 0;
		for (unsigned int j = 0; j < fieldList.size(); ++j)
			if (fieldList[j]->is_extension()) {
				num = fieldList[j]->number();
				break;
			}
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
		sendProtocolItem(preResponseQueue[i].first, *preResponseQueue[i].second);
	
	Response response;
	response.set_cmd_id(cont.cmd_id());
	response.set_response_code(finalResponseCode);
	::google::protobuf::Message *responseExtension = responseContainer.getResponseExtension();
	if (responseExtension)
		response.GetReflection()->MutableMessage(&response, responseExtension->GetDescriptor()->FindExtensionByName("ext"))->CopyFrom(*responseExtension);
	sendProtocolItem(response);
	
	const QList<QPair<ServerMessage::MessageType, ::google::protobuf::Message *> > &postResponseQueue = responseContainer.getPostResponseQueue();
	for (int i = 0; i < postResponseQueue.size(); ++i)
		sendProtocolItem(postResponseQueue[i].first, *postResponseQueue[i].second);
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
		deleteLater();
	++timeRunning;
}

QPair<Server_Game *, Server_Player *> Server_ProtocolHandler::getGame(int gameId) const
{
	if (games.contains(gameId))
		return games.value(gameId);
	return QPair<Server_Game *, Server_Player *>(0, 0);
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
	
	server->serverMutex.lock();
	
	QMapIterator<int, Server_Room *> roomIterator(server->getRooms());
	QMutexLocker gameListLocker(&gameListMutex);
	while (roomIterator.hasNext()) {
		Server_Room *room = roomIterator.next().value();
		room->roomMutex.lock();
		QMapIterator<int, Server_Game *> gameIterator(room->getGames());
		while (gameIterator.hasNext()) {
			Server_Game *game = gameIterator.next().value();
			QMutexLocker gameLocker(&game->gameMutex);
			const QList<Server_Player *> &gamePlayers = game->getPlayers().values();
			for (int j = 0; j < gamePlayers.size(); ++j)
				if (gamePlayers[j]->getUserInfo()->name() == userInfo->name()) {
					gamePlayers[j]->setProtocolHandler(this);
					games.insert(game->getGameId(), QPair<Server_Game *, Server_Player *>(game, gamePlayers[j]));
					
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
					QListIterator<ServerInfo_Player> gameStateIterator(game->getGameState(gamePlayers[j]));
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
		room->roomMutex.unlock();
	}
	server->serverMutex.unlock();
	
	rc.setResponseExtension(re);
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdMessage(const Command_Message &cmd, ResponseContainer &rc)
{
	if (authState == NotLoggedIn)
		return Response::RespLoginNeeded;
	
	server->serverMutex.lock();
	QString receiver = QString::fromStdString(cmd.user_name());
	Server_ProtocolHandler *userHandler = server->getUsers().value(receiver);
	if (!userHandler)
		return Response::RespNameNotFound;
	if (server->isInIgnoreList(receiver, QString::fromStdString(userInfo->name())))
		return Response::RespInIgnoreList;
	
	Event_UserMessage event;
	event.set_sender_name(userInfo->name());
	event.set_receiver_name(cmd.user_name());
	event.set_message(cmd.message());
	
	SessionEvent *se = prepareSessionEvent(event);
	userHandler->sendProtocolItem(*se);
	rc.enqueuePreResponseItem(ServerMessage::SESSION_EVENT, se);
	server->serverMutex.unlock();
	
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdGetGamesOfUser(const Command_GetGamesOfUser &cmd, ResponseContainer &rc)
{
	if (authState == NotLoggedIn)
		return Response::RespLoginNeeded;
	
	server->serverMutex.lock();
	if (!server->getUsers().contains(QString::fromStdString(cmd.user_name())))
		return Response::RespNameNotFound;
	
	Response_GetGamesOfUser *re = new Response_GetGamesOfUser;
	QMapIterator<int, Server_Room *> roomIterator(server->getRooms());
	while (roomIterator.hasNext()) {
		Server_Room *room = roomIterator.next().value();
		room->roomMutex.lock();
		re->add_room_list()->CopyFrom(room->getInfo(false, true));
		QListIterator<ServerInfo_Game> gameIterator(room->getGamesOfUser(QString::fromStdString(cmd.user_name())));
		while (gameIterator.hasNext())
			re->add_game_list()->CopyFrom(gameIterator.next());
		room->roomMutex.unlock();
	}
	server->serverMutex.unlock();
	
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
		server->serverMutex.lock();
		Server_ProtocolHandler *handler = server->getUsers().value(userName);
		if (!handler)
			return Response::RespNameNotFound;
		re->mutable_user_info()->CopyFrom(handler->copyUserInfo(true, userInfo->user_level() & ServerInfo_User::IsModerator));
		server->serverMutex.unlock();
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
	
	Server_Room *r = server->getRooms().value(cmd.room_id(), 0);
	if (!r)
		return Response::RespNameNotFound;
	
	QMutexLocker serverLocker(&server->serverMutex);
	QMutexLocker roomLocker(&r->roomMutex);
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
	server->serverMutex.lock();
	QMapIterator<QString, Server_ProtocolHandler *> userIterator = server->getUsers();
	while (userIterator.hasNext())
		re->add_user_list()->CopyFrom(userIterator.next().value()->copyUserInfo(false));
	server->serverMutex.unlock();
	
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
	
	room->say(this, msg);
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdCreateGame(const Command_CreateGame &cmd, Server_Room *room, ResponseContainer &rc)
{
	if (authState == NotLoggedIn)
		return Response::RespLoginNeeded;

	if (server->getMaxGamesPerUser() > 0)
		if (room->getGamesCreatedByUser(getUserName()) >= server->getMaxGamesPerUser())
			return Response::RespContextError;
	
	QList<int> gameTypes;
	for (int i = cmd.game_type_ids_size() - 1; i >= 0; --i)
		gameTypes.append(cmd.game_type_ids(i));
	
	QString description = QString::fromStdString(cmd.description());
	if (description.size() > 60)
		description = description.left(60);
	Server_Game *game = room->createGame(description, QString::fromStdString(cmd.password()), cmd.max_players(), gameTypes, cmd.only_buddies(), cmd.only_registered(), cmd.spectators_allowed(), cmd.spectators_need_password(), cmd.spectators_can_talk(), cmd.spectators_see_everything(), this);
	
	Server_Player *creator = game->getPlayers().values().first();
	
	QMutexLocker gameListLocker(&gameListMutex);
	games.insert(game->getGameId(), QPair<Server_Game *, Server_Player *>(game, creator));
	
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
	
	QMutexLocker gameListLocker(&gameListMutex);
	
	if (games.contains(cmd.game_id()))
		return Response::RespContextError;
	
	Server_Game *g = room->getGames().value(cmd.game_id());
	if (!g)
		return Response::RespNameNotFound;
	
	QMutexLocker locker(&g->gameMutex);
	
	Response::ResponseCode result = g->checkJoin(userInfo, QString::fromStdString(cmd.password()), cmd.spectator(), cmd.override_restrictions());
	if (result == Response::RespOk) {
		Server_Player *player = g->addPlayer(this, cmd.spectator());
		games.insert(cmd.game_id(), QPair<Server_Game *, Server_Player *>(g, player));
		
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
	return result;
}

Response::ResponseCode Server_ProtocolHandler::cmdLeaveGame(const Command_LeaveGame & /*cmd*/, Server_Game *game, Server_Player *player, ResponseContainer & /*rc*/, GameEventStorage & /*ges*/)
{
	game->removePlayer(player);
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdKickFromGame(const Command_KickFromGame &cmd, Server_Game *game, Server_Player *player, ResponseContainer & /*rc*/, GameEventStorage & /*ges*/)
{
	if ((game->getHostId() != player->getPlayerId()) && !(userInfo->user_level() & ServerInfo_User::IsModerator))
		return Response::RespFunctionNotAllowed;
	
	if (!game->kickPlayer(cmd.player_id()))
		return Response::RespNameNotFound;
	
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdDeckSelect(const Command_DeckSelect &cmd, Server_Game * /*game*/, Server_Player *player, ResponseContainer &rc, GameEventStorage &ges)
{
	if (player->getSpectator())
		return Response::RespFunctionNotAllowed;
	
	DeckList *deck;
	if (cmd.has_deck_id()) {
		try {
			deck = getDeckFromDatabase(cmd.deck_id());
		} catch(Response::ResponseCode r) {
			return r;
		}
	} else
		deck = new DeckList(QString::fromStdString(cmd.deck()));
	
	player->setDeck(deck);
	
	Event_PlayerPropertiesChanged event;
	event.mutable_player_properties()->set_deck_hash(deck->getDeckHash().toStdString());
	ges.enqueueGameEvent(event, player->getPlayerId());
	
	Context_DeckSelect context;
	context.set_deck_hash(deck->getDeckHash().toStdString());
	ges.setGameEventContext(context);
	
	Response_DeckDownload *re = new Response_DeckDownload;
	re->set_deck(deck->writeToString_Native().toStdString());
	
	rc.setResponseExtension(re);
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdSetSideboardPlan(const Command_SetSideboardPlan &cmd, Server_Game * /*game*/, Server_Player *player, ResponseContainer & /*rc*/, GameEventStorage & /*ges*/)
{
	if (player->getSpectator())
		return Response::RespFunctionNotAllowed;
	if (player->getReadyStart())
		return Response::RespContextError;
	
	DeckList *deck = player->getDeck();
	if (!deck)
		return Response::RespContextError;
	
	QList<MoveCard_ToZone> sideboardPlan;
	for (int i = 0; i < cmd.move_list_size(); ++i)
		sideboardPlan.append(cmd.move_list(i));
	deck->setCurrentSideboardPlan(sideboardPlan);
	
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdConcede(const Command_Concede & /*cmd*/, Server_Game *game, Server_Player *player, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
	if (player->getSpectator())
		return Response::RespFunctionNotAllowed;
	if (!game->getGameStarted())
		return Response::RespGameNotStarted;
	if (player->getConceded())
		return Response::RespContextError;
	
	player->setConceded(true);
	game->removeArrowsToPlayer(ges, player);
	game->unattachCards(ges, player);
	player->clearZones();
	
	Event_PlayerPropertiesChanged event;
	event.mutable_player_properties()->set_conceded(true);
	ges.enqueueGameEvent(event, player->getPlayerId());
	ges.setGameEventContext(Context_Concede());
	
	game->stopGameIfFinished();
	if (game->getGameStarted() && (game->getActivePlayer() == player->getPlayerId()))
		game->nextTurn();
	
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdReadyStart(const Command_ReadyStart &cmd, Server_Game *game, Server_Player *player, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
	if (player->getSpectator())
		return Response::RespFunctionNotAllowed;
	
	if (!player->getDeck() || game->getGameStarted())
		return Response::RespContextError;

	if (player->getReadyStart() == cmd.ready())
		return Response::RespContextError;
	
	player->setReadyStart(cmd.ready());
	
	Event_PlayerPropertiesChanged event;
	event.mutable_player_properties()->set_ready_start(cmd.ready());
	ges.enqueueGameEvent(event, player->getPlayerId());
	ges.setGameEventContext(Context_ReadyStart());
	
	if (cmd.ready())
		game->startGameIfReady();
	
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdGameSay(const Command_GameSay &cmd, Server_Game *game, Server_Player *player, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
	if (player->getSpectator() && !game->getSpectatorsCanTalk() && !(userInfo->user_level() & ServerInfo_User::IsModerator))
		return Response::RespFunctionNotAllowed;
	
	Event_GameSay event;
	event.set_message(cmd.message());
	ges.enqueueGameEvent(event, player->getPlayerId());
	
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdShuffle(const Command_Shuffle & /*cmd*/, Server_Game *game, Server_Player *player, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
	if (player->getSpectator())
		return Response::RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return Response::RespGameNotStarted;
	if (player->getConceded())
		return Response::RespContextError;
		
	player->getZones().value("deck")->shuffle();
	
	Event_Shuffle event;
	event.set_zone_name("deck");
	ges.enqueueGameEvent(event, player->getPlayerId());
	
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdMulligan(const Command_Mulligan & /*cmd*/, Server_Game *game, Server_Player *player, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
	if (player->getSpectator())
		return Response::RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return Response::RespGameNotStarted;
	if (player->getConceded())
		return Response::RespContextError;
	
	Server_CardZone *hand = player->getZones().value("hand");
	int number = (hand->cards.size() <= 1) ? player->getInitialCards() : hand->cards.size() - 1;
		
	Server_CardZone *deck = player->getZones().value("deck");
	while (!hand->cards.isEmpty()) {
		CardToMove *cardToMove = new CardToMove;
		cardToMove->set_card_id(hand->cards.first()->getId());
		player->moveCard(ges, hand, QList<const CardToMove *>() << cardToMove, deck, 0, 0, false);
		delete cardToMove;
	}
	
	deck->shuffle();
	ges.enqueueGameEvent(Event_Shuffle(), player->getPlayerId());

	player->drawCards(ges, number);
	
	if (number == player->getInitialCards())
		number = -1;
	
	Context_Mulligan context;
	context.set_number(number);
	ges.setGameEventContext(context);

	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdRollDie(const Command_RollDie &cmd, Server_Game * /*game*/, Server_Player *player, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
	if (player->getSpectator())
		return Response::RespFunctionNotAllowed;
	if (player->getConceded())
		return Response::RespContextError;
	
	Event_RollDie event;
	event.set_sides(cmd.sides());
	event.set_value(rng->getNumber(1, cmd.sides()));
	ges.enqueueGameEvent(event, player->getPlayerId());
	
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdDrawCards(const Command_DrawCards &cmd, Server_Game *game, Server_Player *player, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
	if (player->getSpectator())
		return Response::RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return Response::RespGameNotStarted;
	if (player->getConceded())
		return Response::RespContextError;
		
	return player->drawCards(ges, cmd.number());
}

Response::ResponseCode Server_ProtocolHandler::cmdUndoDraw(const Command_UndoDraw & /*cmd*/, Server_Game *game, Server_Player *player, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
	if (player->getSpectator())
		return Response::RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return Response::RespGameNotStarted;
	if (player->getConceded())
		return Response::RespContextError;
		
	return player->undoDraw(ges);
}

Response::ResponseCode Server_ProtocolHandler::cmdMoveCard(const Command_MoveCard &cmd, Server_Game *game, Server_Player *player, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
	if (player->getSpectator())
		return Response::RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return Response::RespGameNotStarted;
	if (player->getConceded())
		return Response::RespContextError;
	
	QList<const CardToMove *> cardsToMove;
	for (int i = 0; i < cmd.cards_to_move().card_size(); ++i)
		cardsToMove.append(&cmd.cards_to_move().card(i));
	
	return player->moveCard(ges, QString::fromStdString(cmd.start_zone()), cardsToMove, cmd.target_player_id(), QString::fromStdString(cmd.target_zone()), cmd.x(), cmd.y());
}

Response::ResponseCode Server_ProtocolHandler::cmdFlipCard(const Command_FlipCard &cmd, Server_Game *game, Server_Player *player, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
	if (player->getSpectator())
		return Response::RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return Response::RespGameNotStarted;
	if (player->getConceded())
		return Response::RespContextError;
	
	Server_CardZone *zone = player->getZones().value(QString::fromStdString(cmd.zone()));
	if (!zone)
		return Response::RespNameNotFound;
	if (!zone->hasCoords())
		return Response::RespContextError;
	
	Server_Card *card = zone->getCard(cmd.card_id());
	if (!card)
		return Response::RespNameNotFound;
	
	const bool faceDown = cmd.face_down();
	if (faceDown == card->getFaceDown())
		return Response::RespContextError;
	
	card->setFaceDown(faceDown);
	
	Event_FlipCard event;
	event.set_zone_name(zone->getName().toStdString());
	event.set_card_id(card->getId());
	event.set_card_name(card->getName().toStdString());
	event.set_face_down(faceDown);
	ges.enqueueGameEvent(event, player->getPlayerId());
	
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdAttachCard(const Command_AttachCard &cmd, Server_Game *game, Server_Player *player, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
	if (player->getSpectator())
		return Response::RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return Response::RespGameNotStarted;
	if (player->getConceded())
		return Response::RespContextError;
		
	Server_CardZone *startzone = player->getZones().value(QString::fromStdString(cmd.start_zone()));
	if (!startzone)
		return Response::RespNameNotFound;
	
	Server_Card *card = startzone->getCard(cmd.card_id());
	if (!card)
		return Response::RespNameNotFound;

	Server_Player *targetPlayer = 0;
	Server_CardZone *targetzone = 0;
	Server_Card *targetCard = 0;
	
	if (cmd.has_target_player_id()) {
		targetPlayer = game->getPlayer(cmd.target_player_id());
		if (!targetPlayer)
			return Response::RespNameNotFound;
	} else if (!card->getParentCard())
		return Response::RespContextError;
	if (targetPlayer)
		targetzone = targetPlayer->getZones().value(QString::fromStdString(cmd.target_zone()));
	if (targetzone) {
		// This is currently enough to make sure cards don't get attached to a card that is not on the table.
		// Possibly a flag will have to be introduced for this sometime.
		if (!targetzone->hasCoords())
			return Response::RespContextError;
		if (cmd.has_target_card_id())
			targetCard = targetzone->getCard(cmd.target_card_id());
		if (targetCard)
			if (targetCard->getParentCard())
				return Response::RespContextError;
	}
	if (!startzone->hasCoords())
		return Response::RespContextError;
	
	// Get all arrows pointing to or originating from the card being attached and delete them.
	QMapIterator<int, Server_Player *> playerIterator(game->getPlayers());
	while (playerIterator.hasNext()) {
		Server_Player *p = playerIterator.next().value();
		QList<Server_Arrow *> arrows = p->getArrows().values();
		QList<Server_Arrow *> toDelete;
		for (int i = 0; i < arrows.size(); ++i) {
			Server_Arrow *a = arrows[i];
			Server_Card *tCard = qobject_cast<Server_Card *>(a->getTargetItem());
			if ((tCard == card) || (a->getStartCard() == card))
				toDelete.append(a);
		}
		for (int i = 0; i < toDelete.size(); ++i) {
			Event_DeleteArrow event;
			event.set_arrow_id(toDelete[i]->getId());
			ges.enqueueGameEvent(event, p->getPlayerId());
			p->deleteArrow(toDelete[i]->getId());
		}
	}

	if (targetCard) {
		// Unattach all cards attached to the card being attached.
		// Make a copy of the list because its contents change during the loop otherwise.
		QList<Server_Card *> attachedList = card->getAttachedCards();
		for (int i = 0; i < attachedList.size(); ++i)
			attachedList[i]->getZone()->getPlayer()->unattachCard(ges, attachedList[i]);
		
		if (targetzone->isColumnStacked(targetCard->getX(), targetCard->getY())) {
			CardToMove *cardToMove = new CardToMove;
			cardToMove->set_card_id(targetCard->getId());
			targetPlayer->moveCard(ges, targetzone, QList<const CardToMove *>() << cardToMove, targetzone, targetzone->getFreeGridColumn(-2, targetCard->getY(), targetCard->getName()), targetCard->getY(), targetCard->getFaceDown());
			delete cardToMove;
		}
		
		card->setParentCard(targetCard);
		card->setCoords(-1, card->getY());
		
		Event_AttachCard event;
		event.set_start_zone(startzone->getName().toStdString());
		event.set_card_id(card->getId());
		event.set_target_player_id(targetPlayer->getPlayerId());
		event.set_target_zone(targetzone->getName().toStdString());
		event.set_target_card_id(targetCard->getId());
		ges.enqueueGameEvent(event, player->getPlayerId());
		
		startzone->fixFreeSpaces(ges);
	} else
		player->unattachCard(ges, card);
	
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdCreateToken(const Command_CreateToken &cmd, Server_Game *game, Server_Player *player, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
	if (player->getSpectator())
		return Response::RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return Response::RespGameNotStarted;
	if (player->getConceded())
		return Response::RespContextError;
		
	Server_CardZone *zone = player->getZones().value(QString::fromStdString(cmd.zone()));
	if (!zone)
		return Response::RespNameNotFound;

	QString cardName = QString::fromStdString(cmd.card_name());
	int x = cmd.x();
	int y = cmd.y();
	if (zone->hasCoords())
		x = zone->getFreeGridColumn(x, y, cardName);
	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;

	Server_Card *card = new Server_Card(cardName, player->newCardId(), x, y);
	card->moveToThread(player->thread());
	card->setPT(QString::fromStdString(cmd.pt()));
	card->setColor(QString::fromStdString(cmd.color()));
	card->setAnnotation(QString::fromStdString(cmd.annotation()));
	card->setDestroyOnZoneChange(cmd.destroy_on_zone_change());
	
	zone->insertCard(card, x, y);
	
	Event_CreateToken event;
	event.set_zone_name(zone->getName().toStdString());
	event.set_card_id(card->getId());
	event.set_card_name(card->getName().toStdString());
	event.set_color(card->getColor().toStdString());
	event.set_pt(card->getPT().toStdString());
	event.set_annotation(card->getAnnotation().toStdString());
	event.set_destroy_on_zone_change(card->getDestroyOnZoneChange());
	event.set_x(x);
	event.set_y(y);
	ges.enqueueGameEvent(event, player->getPlayerId());
	
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdCreateArrow(const Command_CreateArrow &cmd, Server_Game *game, Server_Player *player, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
	if (player->getSpectator())
		return Response::RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return Response::RespGameNotStarted;
	if (player->getConceded())
		return Response::RespContextError;
	
	Server_Player *startPlayer = game->getPlayer(cmd.start_player_id());
	Server_Player *targetPlayer = game->getPlayer(cmd.target_player_id());
	if (!startPlayer || !targetPlayer)
		return Response::RespNameNotFound;
	QString startZoneName = QString::fromStdString(cmd.start_zone());
	Server_CardZone *startZone = startPlayer->getZones().value(startZoneName);
	bool playerTarget = !cmd.has_target_zone();
	Server_CardZone *targetZone = 0;
	if (!playerTarget)
		targetZone = targetPlayer->getZones().value(QString::fromStdString(cmd.target_zone()));
	if (!startZone || (!targetZone && !playerTarget))
		return Response::RespNameNotFound;
	if (startZone->getType() != ServerInfo_Zone::PublicZone)
		return Response::RespContextError;
	Server_Card *startCard = startZone->getCard(cmd.start_card_id());
	if (!startCard)
		return Response::RespNameNotFound;
	Server_Card *targetCard = 0;
	if (!playerTarget) {
		if (targetZone->getType() != ServerInfo_Zone::PublicZone)
			return Response::RespContextError;
		targetCard = targetZone->getCard(cmd.target_card_id());
	}
	
	Server_ArrowTarget *targetItem;
	if (playerTarget)
		targetItem = targetPlayer;
	else
		targetItem = targetCard;
	if (!targetItem)
		return Response::RespNameNotFound;

	QMapIterator<int, Server_Arrow *> arrowIterator(player->getArrows());
	while (arrowIterator.hasNext()) {
		Server_Arrow *temp = arrowIterator.next().value();
		if ((temp->getStartCard() == startCard) && (temp->getTargetItem() == targetItem))
			return Response::RespContextError;
	}
	
	Server_Arrow *arrow = new Server_Arrow(player->newArrowId(), startCard, targetItem, cmd.arrow_color());
	player->addArrow(arrow);
	
	Event_CreateArrow event;
	ServerInfo_Arrow *arrowInfo = event.mutable_arrow_info();
	arrowInfo->set_id(arrow->getId());
	arrowInfo->set_start_player_id(startPlayer->getPlayerId());
	arrowInfo->set_start_zone(startZoneName.toStdString());
	arrowInfo->set_start_card_id(startCard->getId());
	arrowInfo->set_target_player_id(targetPlayer->getPlayerId());
	if (!playerTarget) {
		arrowInfo->set_target_zone(cmd.target_zone());
		arrowInfo->set_target_card_id(cmd.target_card_id());
	}
	arrowInfo->mutable_arrow_color()->CopyFrom(cmd.arrow_color());
	ges.enqueueGameEvent(event, player->getPlayerId());
	
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdDeleteArrow(const Command_DeleteArrow &cmd, Server_Game *game, Server_Player *player, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
	if (player->getSpectator())
		return Response::RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return Response::RespGameNotStarted;
	if (player->getConceded())
		return Response::RespContextError;
	
	if (!player->deleteArrow(cmd.arrow_id()))
		return Response::RespNameNotFound;
	
	Event_DeleteArrow event;
	event.set_arrow_id(cmd.arrow_id());
	ges.enqueueGameEvent(event, player->getPlayerId());
	
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdSetCardAttr(const Command_SetCardAttr &cmd, Server_Game *game, Server_Player *player, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
	if (player->getSpectator())
		return Response::RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return Response::RespGameNotStarted;
	if (player->getConceded())
		return Response::RespContextError;
	
	return player->setCardAttrHelper(ges, QString::fromStdString(cmd.zone()), cmd.card_id(), cmd.attribute(), QString::fromStdString(cmd.attr_value()));
}

Response::ResponseCode Server_ProtocolHandler::cmdSetCardCounter(const Command_SetCardCounter &cmd, Server_Game *game, Server_Player *player, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
	if (player->getSpectator())
		return Response::RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return Response::RespGameNotStarted;
	if (player->getConceded())
		return Response::RespContextError;
	
	Server_CardZone *zone = player->getZones().value(QString::fromStdString(cmd.zone()));
	if (!zone)
		return Response::RespNameNotFound;
	if (!zone->hasCoords())
		return Response::RespContextError;

	Server_Card *card = zone->getCard(cmd.card_id());
	if (!card)
		return Response::RespNameNotFound;
	
	card->setCounter(cmd.counter_id(), cmd.counter_value());
	
	Event_SetCardCounter event;
	event.set_zone_name(zone->getName().toStdString());
	event.set_card_id(card->getId());
	event.set_counter_id(cmd.counter_id());
	event.set_counter_value(cmd.counter_value());
	ges.enqueueGameEvent(event, player->getPlayerId());
	
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdIncCardCounter(const Command_IncCardCounter &cmd, Server_Game *game, Server_Player *player, ResponseContainer &/*rc*/, GameEventStorage &ges)
{
	if (player->getSpectator())
		return Response::RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return Response::RespGameNotStarted;
	if (player->getConceded())
		return Response::RespContextError;
	
	Server_CardZone *zone = player->getZones().value(QString::fromStdString(cmd.zone()));
	if (!zone)
		return Response::RespNameNotFound;
	if (!zone->hasCoords())
		return Response::RespContextError;

	Server_Card *card = zone->getCard(cmd.card_id());
	if (!card)
		return Response::RespNameNotFound;
	
	int newValue = card->getCounter(cmd.counter_id()) + cmd.counter_delta();
	card->setCounter(cmd.counter_id(), newValue);
	
	Event_SetCardCounter event;
	event.set_zone_name(zone->getName().toStdString());
	event.set_card_id(card->getId());
	event.set_counter_id(cmd.counter_id());
	event.set_counter_value(newValue);
	ges.enqueueGameEvent(event, player->getPlayerId());
	
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdIncCounter(const Command_IncCounter &cmd, Server_Game *game, Server_Player *player, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
	if (player->getSpectator())
		return Response::RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return Response::RespGameNotStarted;
	if (player->getConceded())
		return Response::RespContextError;
	
	const QMap<int, Server_Counter *> counters = player->getCounters();
	Server_Counter *c = counters.value(cmd.counter_id(), 0);
	if (!c)
		return Response::RespNameNotFound;
	
	c->setCount(c->getCount() + cmd.delta());
	
	Event_SetCounter event;
	event.set_counter_id(c->getId());
	event.set_value(c->getCount());
	ges.enqueueGameEvent(event, player->getPlayerId());
	
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdCreateCounter(const Command_CreateCounter &cmd, Server_Game *game, Server_Player *player, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
	if (player->getSpectator())
		return Response::RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return Response::RespGameNotStarted;
	if (player->getConceded())
		return Response::RespContextError;
	
	Server_Counter *c = new Server_Counter(player->newCounterId(), QString::fromStdString(cmd.counter_name()), cmd.counter_color(), cmd.radius(), cmd.value());
	player->addCounter(c);
	
	Event_CreateCounter event;
	ServerInfo_Counter *counterInfo = event.mutable_counter_info();
	counterInfo->set_id(c->getId());
	counterInfo->set_name(c->getName().toStdString());
	counterInfo->mutable_counter_color()->CopyFrom(cmd.counter_color());
	counterInfo->set_radius(c->getRadius());
	counterInfo->set_count(c->getCount());
	ges.enqueueGameEvent(event, player->getPlayerId());
	
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdSetCounter(const Command_SetCounter &cmd, Server_Game *game, Server_Player *player, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
	if (player->getSpectator())
		return Response::RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return Response::RespGameNotStarted;
	if (player->getConceded())
		return Response::RespContextError;
	
	Server_Counter *c = player->getCounters().value(cmd.counter_id(), 0);;
	if (!c)
		return Response::RespNameNotFound;
	
	c->setCount(cmd.value());
	
	Event_SetCounter event;
	event.set_counter_id(c->getId());
	event.set_value(c->getCount());
	ges.enqueueGameEvent(event, player->getPlayerId());
	
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdDelCounter(const Command_DelCounter &cmd, Server_Game *game, Server_Player *player, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
	if (player->getSpectator())
		return Response::RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return Response::RespGameNotStarted;
	if (player->getConceded())
		return Response::RespContextError;
	
	if (!player->deleteCounter(cmd.counter_id()))
		return Response::RespNameNotFound;
	
	Event_DelCounter event;
	event.set_counter_id(cmd.counter_id());
	ges.enqueueGameEvent(event, player->getPlayerId());
	
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdNextTurn(const Command_NextTurn & /*cmd*/, Server_Game *game, Server_Player *player, ResponseContainer & /*rc*/, GameEventStorage & /*ges*/)
{
	if (player->getSpectator())
		return Response::RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return Response::RespGameNotStarted;
	if (player->getConceded())
		return Response::RespContextError;
	
	game->nextTurn();
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdSetActivePhase(const Command_SetActivePhase &cmd, Server_Game *game, Server_Player *player, ResponseContainer & /*rc*/, GameEventStorage & /*ges*/)
{
	if (player->getSpectator())
		return Response::RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return Response::RespGameNotStarted;
	if (player->getConceded())
		return Response::RespContextError;
	
	if (game->getActivePlayer() != player->getPlayerId())
		return Response::RespContextError;
	game->setActivePhase(cmd.phase());
	
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdDumpZone(const Command_DumpZone &cmd, Server_Game *game, Server_Player *player, ResponseContainer &rc, GameEventStorage &ges)
{
	if (!game->getGameStarted())
		return Response::RespGameNotStarted;
	
	Server_Player *otherPlayer = game->getPlayer(cmd.player_id());
	if (!otherPlayer)
		return Response::RespNameNotFound;
	Server_CardZone *zone = otherPlayer->getZones().value(QString::fromStdString(cmd.zone_name()));
	if (!zone)
		return Response::RespNameNotFound;
	if (!((zone->getType() == ServerInfo_Zone::PublicZone) || (player == otherPlayer)))
		return Response::RespContextError;
	
	int numberCards = cmd.number_cards();
	
	Response_DumpZone *re = new Response_DumpZone;
	ServerInfo_Zone *zoneInfo = re->mutable_zone_info();
	zoneInfo->set_name(zone->getName().toStdString());
	zoneInfo->set_type(zone->getType());
	zoneInfo->set_with_coords(zone->hasCoords());
	zoneInfo->set_card_count(numberCards < zone->cards.size() ? zone->cards.size() : numberCards);
	
	for (int i = 0; (i < zone->cards.size()) && (i < numberCards || numberCards == -1); ++i) {
		Server_Card *card = zone->cards[i];
		QString displayedName = card->getFaceDown() ? QString() : card->getName();
		ServerInfo_Card *cardInfo = zoneInfo->add_card_list();
		cardInfo->set_name(displayedName.toStdString());
		if (zone->getType() == ServerInfo_Zone::HiddenZone)
			cardInfo->set_id(i);
		else {
			cardInfo->set_id(card->getId());
			cardInfo->set_x(card->getX());
			cardInfo->set_y(card->getY());
			cardInfo->set_face_down(card->getFaceDown());
			cardInfo->set_tapped(card->getTapped());
			cardInfo->set_attacking(card->getAttacking());
			cardInfo->set_color(card->getColor().toStdString());
			cardInfo->set_pt(card->getPT().toStdString());
			cardInfo->set_annotation(card->getAnnotation().toStdString());
			cardInfo->set_destroy_on_zone_change(card->getDestroyOnZoneChange());
			cardInfo->set_doesnt_untap(card->getDoesntUntap());
			
			QMapIterator<int, int> cardCounterIterator(card->getCounters());
			while (cardCounterIterator.hasNext()) {
				cardCounterIterator.next();
				ServerInfo_CardCounter *counterInfo = cardInfo->add_counter_list();
				counterInfo->set_id(cardCounterIterator.key());
				counterInfo->set_value(cardCounterIterator.value());
			}

			if (card->getParentCard()) {
				cardInfo->set_attach_player_id(card->getParentCard()->getZone()->getPlayer()->getPlayerId());
				cardInfo->set_attach_zone(card->getParentCard()->getZone()->getName().toStdString());
				cardInfo->set_attach_card_id(card->getParentCard()->getId());
			}
		}
	}
	if (zone->getType() == ServerInfo_Zone::HiddenZone) {
		zone->setCardsBeingLookedAt(numberCards);
		
		Event_DumpZone event;
		event.set_zone_owner_id(otherPlayer->getPlayerId());
		event.set_zone_name(zone->getName().toStdString());
		event.set_number_cards(numberCards);
		ges.enqueueGameEvent(event, player->getPlayerId());
	}
	rc.setResponseExtension(re);
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdStopDumpZone(const Command_StopDumpZone &cmd, Server_Game *game, Server_Player *player, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
	if (!game->getGameStarted())
		return Response::RespGameNotStarted;
	if (player->getConceded())
		return Response::RespContextError;
	
	Server_Player *otherPlayer = game->getPlayer(cmd.player_id());
	if (!otherPlayer)
		return Response::RespNameNotFound;
	Server_CardZone *zone = otherPlayer->getZones().value(QString::fromStdString(cmd.zone_name()));
	if (!zone)
		return Response::RespNameNotFound;
	
	if (zone->getType() == ServerInfo_Zone::HiddenZone) {
		zone->setCardsBeingLookedAt(0);
		
		Event_StopDumpZone event;
		event.set_zone_owner_id(cmd.player_id());
		event.set_zone_name(zone->getName().toStdString());
		ges.enqueueGameEvent(event, player->getPlayerId());
	}
	return Response::RespOk;
}

Response::ResponseCode Server_ProtocolHandler::cmdRevealCards(const Command_RevealCards &cmd, Server_Game *game, Server_Player *player, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
	if (player->getSpectator())
		return Response::RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return Response::RespGameNotStarted;
	if (player->getConceded())
		return Response::RespContextError;
	
	Server_Player *otherPlayer = 0;
	if (cmd.has_player_id()) {
		otherPlayer = game->getPlayer(cmd.player_id());
		if (!otherPlayer)
			return Response::RespNameNotFound;
	}
	Server_CardZone *zone = player->getZones().value(QString::fromStdString(cmd.zone_name()));
	if (!zone)
		return Response::RespNameNotFound;
	
	QList<Server_Card *> cardsToReveal;
	if (!cmd.has_card_id())
		cardsToReveal = zone->cards;
	else if (cmd.card_id() == -2) {
		if (zone->cards.isEmpty())
			return Response::RespContextError;
		cardsToReveal.append(zone->cards.at(rng->getNumber(0, zone->cards.size() - 1)));
	} else {
		Server_Card *card = zone->getCard(cmd.card_id());
		if (!card)
			return Response::RespNameNotFound;
		cardsToReveal.append(card);
	}
	
	Event_RevealCards eventOthers;
	eventOthers.set_zone_name(zone->getName().toStdString());
	if (cmd.has_card_id())
		eventOthers.set_card_id(cmd.card_id());
	if (cmd.has_player_id())
		eventOthers.set_other_player_id(cmd.player_id());
	
	Event_RevealCards eventPrivate(eventOthers);
	
	for (int i = 0; i < cardsToReveal.size(); ++i) {
		Server_Card *card = cardsToReveal[i];
		ServerInfo_Card *cardInfo = eventPrivate.add_cards();

		cardInfo->set_id(card->getId());
		cardInfo->set_name(card->getName().toStdString());
		cardInfo->set_x(card->getX());
		cardInfo->set_y(card->getY());
		cardInfo->set_face_down(card->getFaceDown());
		cardInfo->set_tapped(card->getTapped());
		cardInfo->set_attacking(card->getAttacking());
		cardInfo->set_color(card->getColor().toStdString());
		cardInfo->set_pt(card->getPT().toStdString());
		cardInfo->set_annotation(card->getAnnotation().toStdString());
		cardInfo->set_destroy_on_zone_change(card->getDestroyOnZoneChange());
		cardInfo->set_doesnt_untap(card->getDoesntUntap());
		
		QMapIterator<int, int> cardCounterIterator(card->getCounters());
		while (cardCounterIterator.hasNext()) {
			cardCounterIterator.next();
			ServerInfo_CardCounter *counterInfo = cardInfo->add_counter_list();
			counterInfo->set_id(cardCounterIterator.key());
			counterInfo->set_value(cardCounterIterator.value());
		}
		
		if (card->getParentCard()) {
			cardInfo->set_attach_player_id(card->getParentCard()->getZone()->getPlayer()->getPlayerId());
			cardInfo->set_attach_zone(card->getParentCard()->getZone()->getName().toStdString());
			cardInfo->set_attach_card_id(card->getParentCard()->getId());
		}
	}
	
	if (cmd.has_player_id()) {
		ges.enqueueGameEvent(eventPrivate, player->getPlayerId(), GameEventStorageItem::SendToPrivate, cmd.player_id());
		ges.enqueueGameEvent(eventOthers, player->getPlayerId(), GameEventStorageItem::SendToOthers);
	} else
		ges.enqueueGameEvent(eventPrivate, player->getPlayerId());
	
	return Response::RespOk;
}
