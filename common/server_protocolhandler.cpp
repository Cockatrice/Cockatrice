#include <QDebug>
#include "rng_abstract.h"
#include "server_protocolhandler.h"
#include "protocol.h"
#include "protocol_items.h"
#include "server_room.h"
#include "server_card.h"
#include "server_arrow.h"
#include "server_cardzone.h"
#include "server_counter.h"
#include "server_game.h"
#include "server_player.h"
#include "decklist.h"
#include <QDateTime>
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
#include "pb/command_deck_upload.pb.h"
#include "pb/command_deck_download.pb.h"
#include "pb/command_deck_new_dir.pb.h"
#include "pb/command_deck_del_dir.pb.h"
#include "pb/command_deck_del.pb.h"
#include <google/protobuf/descriptor.h>

Server_ProtocolHandler::Server_ProtocolHandler(Server *_server, QObject *parent)
	: QObject(parent), server(_server), authState(PasswordWrong), acceptsUserListChanges(false), acceptsRoomListChanges(false), userInfo(0), sessionId(-1), timeRunning(0), lastDataReceived(0), gameListMutex(QMutex::Recursive)
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
		
		if ((authState == UnknownUser) || p->getSpectator())
			g->removePlayer(p);
		else {
			p->setProtocolHandler(0);
			g->postConnectionStatusUpdate(p, false);
		}
	}
	gameListMutex.unlock();

	delete userInfo;
	QMapIterator<QString, ServerInfo_User *> i(buddyList);
	while (i.hasNext())
		delete i.next().value();
	QMapIterator<QString, ServerInfo_User *> j(ignoreList);
	while (j.hasNext())
		delete j.next().value();
}

void Server_ProtocolHandler::playerRemovedFromGame(Server_Game *game)
{
	qDebug() << "Server_ProtocolHandler::playerRemovedFromGame(): gameId =" << game->getGameId();
	
	QMutexLocker locker(&gameListMutex);
	games.remove(game->getGameId());
}

ResponseCode Server_ProtocolHandler::processSessionCommandContainer(CommandContainer *cont, BlaContainer *bla)
{
	ResponseCode finalResponseCode = RespOk;
	for (int i = cont->session_command_size() - 1; i >= 0; --i) {
		ResponseCode resp = RespInvalidCommand;
		const SessionCommand &sc = cont->session_command(i);
		std::vector< const ::google::protobuf::FieldDescriptor * > fieldList;
		sc.GetReflection()->ListFields(sc, &fieldList);
		int num = 0;
		for (unsigned int j = 0; j < fieldList.size(); ++j)
			if (fieldList[j]->is_extension()) {
				num = fieldList[j]->number();
				break;
			}
		switch ((SessionCommand::SessionCommandType) num) {
			case SessionCommand::PING: resp = cmdPing(sc.GetExtension(Command_Ping::ext), cont); break;
			case SessionCommand::LOGIN: resp = cmdLogin(sc.GetExtension(Command_Login::ext), cont, bla); break;
			case SessionCommand::MESSAGE: resp = cmdMessage(sc.GetExtension(Command_Message::ext), cont, bla); break;
			case SessionCommand::ADD_TO_LIST: resp = cmdAddToList(sc.GetExtension(Command_AddToList::ext), cont); break;
			case SessionCommand::REMOVE_FROM_LIST: resp = cmdRemoveFromList(sc.GetExtension(Command_RemoveFromList::ext), cont); break;
			case SessionCommand::DECK_LIST: resp = cmdDeckList(sc.GetExtension(Command_DeckList::ext), cont); break;
			case SessionCommand::DECK_NEW_DIR: resp = cmdDeckNewDir(sc.GetExtension(Command_DeckNewDir::ext), cont); break;
			case SessionCommand::DECK_DEL_DIR: resp = cmdDeckDelDir(sc.GetExtension(Command_DeckDelDir::ext), cont); break;
			case SessionCommand::DECK_DEL: resp = cmdDeckDel(sc.GetExtension(Command_DeckDel::ext), cont); break;
			case SessionCommand::DECK_UPLOAD: resp = cmdDeckUpload(sc.GetExtension(Command_DeckUpload::ext), cont); break;
			case SessionCommand::DECK_DOWNLOAD: resp = cmdDeckDownload(sc.GetExtension(Command_DeckDownload::ext), cont); break;
			case SessionCommand::GET_GAMES_OF_USER: resp = cmdGetGamesOfUser(sc.GetExtension(Command_GetGamesOfUser::ext), cont, bla); break;
			case SessionCommand::GET_USER_INFO: resp = cmdGetUserInfo(sc.GetExtension(Command_GetUserInfo::ext), cont, bla); break;
			case SessionCommand::LIST_ROOMS: resp = cmdListRooms(sc.GetExtension(Command_ListRooms::ext), cont, bla); break;
			case SessionCommand::JOIN_ROOM: resp = cmdJoinRoom(sc.GetExtension(Command_JoinRoom::ext), cont, bla); break;
			case SessionCommand::LIST_USERS: resp = cmdListUsers(sc.GetExtension(Command_ListUsers::ext), cont, bla); break;
		}
		if ((resp != RespOk) && (resp != RespNothing))
			finalResponseCode = resp;
	}
	return finalResponseCode;
}

ResponseCode Server_ProtocolHandler::processRoomCommandContainer(CommandContainer *cont, BlaContainer *bla)
{
	if (authState == PasswordWrong)
		return RespLoginNeeded;

	Server_Room *room = rooms.value(cont->room_id(), 0);
	if (!room)
		return RespNotInRoom;
	
	QMutexLocker locker(&room->roomMutex);

	ResponseCode finalResponseCode = RespOk;
	for (int i = cont->room_command_size() - 1; i >= 0; --i) {
		ResponseCode resp = RespInvalidCommand;
		const RoomCommand &sc = cont->room_command(i);
		std::vector< const ::google::protobuf::FieldDescriptor * > fieldList;
		sc.GetReflection()->ListFields(sc, &fieldList);
		int num = 0;
		for (unsigned int j = 0; j < fieldList.size(); ++j)
			if (fieldList[j]->is_extension()) {
				num = fieldList[j]->number();
				break;
			}
		switch ((RoomCommand::RoomCommandType) num) {
			case RoomCommand::LEAVE_ROOM: resp = cmdLeaveRoom(sc.GetExtension(Command_LeaveRoom::ext), cont, room); break;
			case RoomCommand::ROOM_SAY: resp = cmdRoomSay(sc.GetExtension(Command_RoomSay::ext), cont, room); break;
			case RoomCommand::CREATE_GAME: resp = cmdCreateGame(sc.GetExtension(Command_CreateGame::ext), cont, room); break;
			case RoomCommand::JOIN_GAME: resp = cmdJoinGame(sc.GetExtension(Command_JoinGame::ext), cont, room); break;
		}
		if ((resp != RespOk) && (resp != RespNothing))
			finalResponseCode = resp;
	}
	return finalResponseCode;
}

ResponseCode Server_ProtocolHandler::processGameCommandContainer(CommandContainer *cont, BlaContainer *bla)
{
	if (authState == PasswordWrong)
		return RespLoginNeeded;
	
	gameListMutex.lock();
	if (!games.contains(cont->game_id())) {
		qDebug() << "invalid game";
		return RespNotInRoom;
	}
	QPair<Server_Game *, Server_Player *> gamePair = games.value(cont->game_id());
	Server_Game *game = gamePair.first;
	Server_Player *player = gamePair.second;
	
	QMutexLocker locker(&game->gameMutex);
	gameListMutex.unlock();

	ResponseCode finalResponseCode = RespOk;
	for (int i = cont->game_command_size() - 1; i >= 0; --i) {
		ResponseCode resp = RespInvalidCommand;
		const GameCommand &sc = cont->game_command(i);
		std::vector< const ::google::protobuf::FieldDescriptor * > fieldList;
		sc.GetReflection()->ListFields(sc, &fieldList);
		int num = 0;
		for (unsigned int j = 0; j < fieldList.size(); ++j)
			if (fieldList[j]->is_extension()) {
				num = fieldList[j]->number();
				break;
			}
		switch ((GameCommand::GameCommandType) num) {
			case GameCommand::KICK_FROM_GAME: resp = cmdKickFromGame(sc.GetExtension(Command_KickFromGame::ext), cont, game, player, bla); break;
			case GameCommand::LEAVE_GAME: resp = cmdLeaveGame(sc.GetExtension(Command_LeaveGame::ext), cont, game, player, bla); break;
			case GameCommand::GAME_SAY: resp = cmdGameSay(sc.GetExtension(Command_GameSay::ext), cont, game, player, bla); break;
			case GameCommand::SHUFFLE: resp = cmdShuffle(sc.GetExtension(Command_Shuffle::ext), cont, game, player, bla); break;
			case GameCommand::MULLIGAN: resp = cmdMulligan(sc.GetExtension(Command_Mulligan::ext), cont, game, player, bla); break;
			case GameCommand::ROLL_DIE: resp = cmdRollDie(sc.GetExtension(Command_RollDie::ext), cont, game, player, bla); break;
			case GameCommand::DRAW_CARDS: resp = cmdDrawCards(sc.GetExtension(Command_DrawCards::ext), cont, game, player, bla); break;
			case GameCommand::UNDO_DRAW: resp = cmdUndoDraw(sc.GetExtension(Command_UndoDraw::ext), cont, game, player, bla); break;
			case GameCommand::FLIP_CARD: resp = cmdFlipCard(sc.GetExtension(Command_FlipCard::ext), cont, game, player, bla); break;
			case GameCommand::ATTACH_CARD: resp = cmdAttachCard(sc.GetExtension(Command_AttachCard::ext), cont, game, player, bla); break;
			case GameCommand::CREATE_TOKEN: resp = cmdCreateToken(sc.GetExtension(Command_CreateToken::ext), cont, game, player, bla); break;
			case GameCommand::CREATE_ARROW: resp = cmdCreateArrow(sc.GetExtension(Command_CreateArrow::ext), cont, game, player, bla); break;
			case GameCommand::DELETE_ARROW: resp = cmdDeleteArrow(sc.GetExtension(Command_DeleteArrow::ext), cont, game, player, bla); break;
			case GameCommand::SET_CARD_ATTR: resp = cmdSetCardAttr(sc.GetExtension(Command_SetCardAttr::ext), cont, game, player, bla); break;
			case GameCommand::SET_CARD_COUNTER: resp = cmdSetCardCounter(sc.GetExtension(Command_SetCardCounter::ext), cont, game, player, bla); break;
			case GameCommand::INC_CARD_COUNTER: resp = cmdIncCardCounter(sc.GetExtension(Command_IncCardCounter::ext), cont, game, player, bla); break;
			case GameCommand::READY_START: resp = cmdReadyStart(sc.GetExtension(Command_ReadyStart::ext), cont, game, player, bla); break;
			case GameCommand::CONCEDE: resp = cmdConcede(sc.GetExtension(Command_Concede::ext), cont, game, player, bla); break;
			case GameCommand::INC_COUNTER: resp = cmdIncCounter(sc.GetExtension(Command_IncCounter::ext), cont, game, player, bla); break;
			case GameCommand::CREATE_COUNTER: resp = cmdCreateCounter(sc.GetExtension(Command_CreateCounter::ext), cont, game, player, bla); break;
			case GameCommand::SET_COUNTER: resp = cmdSetCounter(sc.GetExtension(Command_SetCounter::ext), cont, game, player, bla); break;
			case GameCommand::DEL_COUNTER: resp = cmdDelCounter(sc.GetExtension(Command_DelCounter::ext), cont, game, player, bla); break;
			case GameCommand::NEXT_TURN: resp = cmdNextTurn(sc.GetExtension(Command_NextTurn::ext), cont, game, player, bla); break;
			case GameCommand::SET_ACTIVE_PHASE: resp = cmdSetActivePhase(sc.GetExtension(Command_SetActivePhase::ext), cont, game, player, bla); break;
			case GameCommand::DUMP_ZONE: resp = cmdDumpZone(sc.GetExtension(Command_DumpZone::ext), cont, game, player, bla); break;
			case GameCommand::STOP_DUMP_ZONE: resp = cmdStopDumpZone(sc.GetExtension(Command_StopDumpZone::ext), cont, game, player, bla); break;
			case GameCommand::REVEAL_CARDS: resp = cmdRevealCards(sc.GetExtension(Command_RevealCards::ext), cont, game, player, bla); break;
			case GameCommand::MOVE_CARD: resp = cmdMoveCard(sc.GetExtension(Command_MoveCard::ext), cont, game, player, bla); break;
			case GameCommand::SET_SIDEBOARD_PLAN: resp = cmdSetSideboardPlan(sc.GetExtension(Command_SetSideboardPlan::ext), cont, game, player, bla); break;
			case GameCommand::DECK_SELECT: resp = cmdDeckSelect(sc.GetExtension(Command_DeckSelect::ext), cont, game, player, bla); break;
		}
		if ((resp != RespOk) && (resp != RespNothing))
			finalResponseCode = resp;
	}
	return finalResponseCode;
}

ResponseCode Server_ProtocolHandler::processModeratorCommandContainer(CommandContainer *cont, BlaContainer *bla)
{
	if (!userInfo)
		return RespLoginNeeded;
	if (!(userInfo->getUserLevel() & ServerInfo_User::IsModerator))
		return RespLoginNeeded;

	ResponseCode finalResponseCode = RespOk;
	for (int i = cont->moderator_command_size() - 1; i >= 0; --i) {
		ResponseCode resp = RespInvalidCommand;
		const ModeratorCommand &sc = cont->moderator_command(i);
		std::vector< const ::google::protobuf::FieldDescriptor * > fieldList;
		sc.GetReflection()->ListFields(sc, &fieldList);
		int num = 0;
		for (unsigned int j = 0; j < fieldList.size(); ++j)
			if (fieldList[j]->is_extension()) {
				num = fieldList[j]->number();
				break;
			}
		switch ((ModeratorCommand::ModeratorCommandType) num) {
			case ModeratorCommand::BAN_FROM_SERVER: resp = cmdBanFromServer(sc.GetExtension(Command_BanFromServer::ext), cont); break;
		}
		if ((resp != RespOk) && (resp != RespNothing))
			finalResponseCode = resp;
	}
	return finalResponseCode;
}

ResponseCode Server_ProtocolHandler::processAdminCommandContainer(CommandContainer *cont, BlaContainer *bla)
{
	if (!userInfo)
		return RespLoginNeeded;
	if (!(userInfo->getUserLevel() & ServerInfo_User::IsAdmin))
		return RespLoginNeeded;

	ResponseCode finalResponseCode = RespOk;
	for (int i = cont->admin_command_size() - 1; i >= 0; --i) {
		ResponseCode resp = RespInvalidCommand;
		const AdminCommand &sc = cont->admin_command(i);
		std::vector< const ::google::protobuf::FieldDescriptor * > fieldList;
		sc.GetReflection()->ListFields(sc, &fieldList);
		int num = 0;
		for (unsigned int j = 0; j < fieldList.size(); ++j)
			if (fieldList[j]->is_extension()) {
				num = fieldList[j]->number();
				break;
			}
		switch ((AdminCommand::AdminCommandType) num) {
			case AdminCommand::SHUTDOWN_SERVER: resp = cmdShutdownServer(sc.GetExtension(Command_ShutdownServer::ext), cont); break;
			case AdminCommand::UPDATE_SERVER_MESSAGE: resp = cmdUpdateServerMessage(sc.GetExtension(Command_UpdateServerMessage::ext), cont); break;
		}
		if ((resp != RespOk) && (resp != RespNothing))
			finalResponseCode = resp;
	}
	return finalResponseCode;
}

void Server_ProtocolHandler::processCommandContainer(CommandContainer *cont)
{
	lastDataReceived = timeRunning;
	
	BlaContainer *bla = new BlaContainer;
	ResponseCode finalResponseCode;
	
	if (cont->game_command_size()) {
		finalResponseCode = processGameCommandContainer(cont, bla);
	} else if (cont->room_command_size()) {
		finalResponseCode = processRoomCommandContainer(cont, bla);
	} else if (cont->session_command_size()) {
		finalResponseCode = processSessionCommandContainer(cont, bla);
	} else if (cont->moderator_command_size()) {
		finalResponseCode = processModeratorCommandContainer(cont, bla);
	} else if (cont->admin_command_size()) {
		finalResponseCode = processAdminCommandContainer(cont, bla);
	}
	
	ProtocolResponse *pr = bla->getResponse();
	if (!pr)
		pr = new ProtocolResponse(cont->cmd_id(), finalResponseCode);
	
	gameListMutex.lock();
	GameEventContainer *gQPublic = bla->getGameEventQueuePublic();
	if (gQPublic) {
		QPair<Server_Game *, Server_Player *> gamePlayerPair = games.value(gQPublic->getGameId());
		if (gamePlayerPair.first) {
			GameEventContainer *gQPrivate = bla->getGameEventQueuePrivate();
			GameEventContainer *gQOmniscient = bla->getGameEventQueueOmniscient();
			if (gQPrivate) {
				int privatePlayerId = bla->getPrivatePlayerId();
				Server_Player *privatePlayer;
				if (privatePlayerId == -1)
					privatePlayer = gamePlayerPair.second;
				else
					privatePlayer = gamePlayerPair.first->getPlayer(privatePlayerId);
				if (gQOmniscient) {
					gamePlayerPair.first->sendGameEventContainer(gQPublic, privatePlayer, true);
					gamePlayerPair.first->sendGameEventContainerOmniscient(gQOmniscient, privatePlayer);
				} else
					gamePlayerPair.first->sendGameEventContainer(gQPublic, privatePlayer);
				privatePlayer->sendProtocolItem(gQPrivate);
			} else
				gamePlayerPair.first->sendGameEventContainer(gQPublic);
		}
	}
	gameListMutex.unlock();
	
	const QList<ProtocolItem *> &iQ = bla->getItemQueue();
	for (int i = 0; i < iQ.size(); ++i)
		sendProtocolItem(iQ[i]);
	
	sendProtocolItem(pr);
	
	while (!itemQueue.isEmpty())
		sendProtocolItem(itemQueue.takeFirst());

//	if (cont->getReceiverMayDelete())
		delete cont;
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

void Server_ProtocolHandler::enqueueProtocolItem(ProtocolItem *item)
{
	itemQueue.append(item);
}

QPair<Server_Game *, Server_Player *> Server_ProtocolHandler::getGame(int gameId) const
{
	if (games.contains(gameId))
		return games.value(gameId);
	return QPair<Server_Game *, Server_Player *>(0, 0);
}

ResponseCode Server_ProtocolHandler::cmdPing(const Command_Ping & /*cmd*/, CommandContainer * /*cont*/)
{
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdLogin(const Command_Login &cmd, CommandContainer *cont, BlaContainer *bla)
{
	QString userName = QString::fromStdString(cmd.user_name()).simplified();
	if (userName.isEmpty() || (userInfo != 0))
		return RespContextError;
	authState = server->loginUser(this, userName, QString::fromStdString(cmd.password()));
	if (authState == PasswordWrong)
		return RespWrongPassword;
	if (authState == WouldOverwriteOldSession)
		return RespWouldOverwriteOldSession;

	ProtocolItem *serverMessage = new Event_ServerMessage(server->getLoginMessage());
	if (getCompressionSupport())
		serverMessage->setCompressed(true);
	enqueueProtocolItem(serverMessage);

	QList<ServerInfo_User *> _buddyList, _ignoreList;
	if (authState == PasswordRight) {
		buddyList = server->getBuddyList(userInfo->getName());
		
		QMapIterator<QString, ServerInfo_User *> buddyIterator(buddyList);
		while (buddyIterator.hasNext())
			_buddyList.append(new ServerInfo_User(buddyIterator.next().value()));
	
		ignoreList = server->getIgnoreList(userInfo->getName());
		
		QMapIterator<QString, ServerInfo_User *> ignoreIterator(ignoreList);
		while (ignoreIterator.hasNext())
			_ignoreList.append(new ServerInfo_User(ignoreIterator.next().value()));
	}
	
	server->serverMutex.lock();
	
	QList<ServerInfo_Game *> gameList;
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
				if (gamePlayers[j]->getUserInfo()->getName() == userInfo->getName()) {
					gamePlayers[j]->setProtocolHandler(this);
					game->postConnectionStatusUpdate(gamePlayers[j], true);
					games.insert(game->getGameId(), QPair<Server_Game *, Server_Player *>(game, gamePlayers[j]));
					
					enqueueProtocolItem(new Event_GameJoined(game->getGameId(), game->getDescription(), game->getHostId(), gamePlayers[j]->getPlayerId(), gamePlayers[j]->getSpectator(), game->getSpectatorsCanTalk(), game->getSpectatorsSeeEverything(), true));
					enqueueProtocolItem(GameEventContainer::makeNew(new Event_GameStateChanged(game->getGameStarted(), game->getActivePlayer(), game->getActivePhase(), game->getGameState(gamePlayers[j])), game->getGameId()));
					
					break;
				}
		}
		room->roomMutex.unlock();
	}
	server->serverMutex.unlock();
	
	ProtocolResponse *resp = new Response_Login(cont->cmd_id(), RespOk, new ServerInfo_User(userInfo, true), _buddyList, _ignoreList);
	if (getCompressionSupport())
		resp->setCompressed(true);
	bla->setResponse(resp);
	return RespNothing;
}

ResponseCode Server_ProtocolHandler::cmdMessage(const Command_Message &cmd, CommandContainer *cont, BlaContainer *bla)
{
	if (authState == PasswordWrong)
		return RespLoginNeeded;
	
	QString receiver = QString::fromStdString(cmd.user_name());
	Server_ProtocolHandler *userHandler = server->getUsers().value(receiver);
	qDebug() << "cmdMessage: recv=" << receiver << (userHandler == 0 ? "not found" : "found");
	if (!userHandler)
		return RespNameNotFound;
	if (userHandler->getIgnoreList().contains(userInfo->getName()))
		return RespInIgnoreList;
	
	QString message = QString::fromStdString(cmd.message());
	bla->enqueueItem(new Event_Message(userInfo->getName(), receiver, message));
	userHandler->sendProtocolItem(new Event_Message(userInfo->getName(), receiver, message));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdGetGamesOfUser(const Command_GetGamesOfUser &cmd, CommandContainer *cont, BlaContainer *bla)
{
	if (authState == PasswordWrong)
		return RespLoginNeeded;
	
	server->serverMutex.lock();
	if (!server->getUsers().contains(QString::fromStdString(cmd.user_name())))
		return RespNameNotFound;
	
	QList<ServerInfo_Room *> roomList;
	QList<ServerInfo_Game *> gameList;
	QMapIterator<int, Server_Room *> roomIterator(server->getRooms());
	while (roomIterator.hasNext()) {
		Server_Room *room = roomIterator.next().value();
		room->roomMutex.lock();
		roomList.append(room->getInfo(false, true));
		gameList << room->getGamesOfUser(QString::fromStdString(cmd.user_name()));
		room->roomMutex.unlock();
	}
	server->serverMutex.unlock();
	
	ProtocolResponse *resp = new Response_GetGamesOfUser(cont->cmd_id(), RespOk, roomList, gameList);
	if (getCompressionSupport())
		resp->setCompressed(true);
	bla->setResponse(resp);
	return RespNothing;
}

ResponseCode Server_ProtocolHandler::cmdGetUserInfo(const Command_GetUserInfo &cmd, CommandContainer *cont, BlaContainer *bla)
{
	if (authState == PasswordWrong)
		return RespLoginNeeded;
	
	QString userName = QString::fromStdString(cmd.user_name());
	ServerInfo_User *result;
	if (userName.isEmpty())
		result = new ServerInfo_User(userInfo);
	else {
		Server_ProtocolHandler *handler = server->getUsers().value(userName);
		if (!handler)
			return RespNameNotFound;
		result = new ServerInfo_User(handler->getUserInfo(), true, userInfo->getUserLevel() & ServerInfo_User::IsModerator);
	}
	
	bla->setResponse(new Response_GetUserInfo(cont->cmd_id(), RespOk, result));
	return RespNothing;
}

ResponseCode Server_ProtocolHandler::cmdListRooms(const Command_ListRooms & /*cmd*/, CommandContainer *cont, BlaContainer *bla)
{
	if (authState == PasswordWrong)
		return RespLoginNeeded;
	
	QList<ServerInfo_Room *> eventRoomList;
	QMapIterator<int, Server_Room *> roomIterator(server->getRooms());
	while (roomIterator.hasNext())
		eventRoomList.append(roomIterator.next().value()->getInfo(false));
	bla->enqueueItem(new Event_ListRooms(eventRoomList));
	
	acceptsRoomListChanges = true;
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdJoinRoom(const Command_JoinRoom &cmd, CommandContainer *cont, BlaContainer *bla)
{
	if (authState == PasswordWrong)
		return RespLoginNeeded;
	
	if (rooms.contains(cmd.room_id()))
		return RespContextError;
	
	Server_Room *r = server->getRooms().value(cmd.room_id(), 0);
	if (!r)
		return RespNameNotFound;
	
	QMutexLocker serverLocker(&server->serverMutex);
	QMutexLocker roomLocker(&r->roomMutex);
	r->addClient(this);
	rooms.insert(r->getId(), r);
	
	enqueueProtocolItem(new Event_RoomSay(r->getId(), QString(), r->getJoinMessage()));
	
	ServerInfo_Room *info = r->getInfo(true);
	if (getCompressionSupport())
		info->setCompressed(true);
	bla->setResponse(new Response_JoinRoom(cont->cmd_id(), RespOk, info));
	return RespNothing;
}

ResponseCode Server_ProtocolHandler::cmdListUsers(const Command_ListUsers & /*cmd*/, CommandContainer *cont, BlaContainer *bla)
{
	if (authState == PasswordWrong)
		return RespLoginNeeded;
	
	QList<ServerInfo_User *> resultList;
	QMapIterator<QString, Server_ProtocolHandler *> userIterator = server->getUsers();
	while (userIterator.hasNext())
		resultList.append(new ServerInfo_User(userIterator.next().value()->getUserInfo(), false));
	
	acceptsUserListChanges = true;

	ProtocolResponse *resp = new Response_ListUsers(cont->cmd_id(), RespOk, resultList);
	if (getCompressionSupport())
		resp->setCompressed(true);
	bla->setResponse(resp);
	return RespNothing;
}

ResponseCode Server_ProtocolHandler::cmdLeaveRoom(const Command_LeaveRoom & /*cmd*/, CommandContainer * /*cont*/, Server_Room *room)
{
	rooms.remove(room->getId());
	room->removeClient(this);
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdRoomSay(const Command_RoomSay &cmd, CommandContainer * /*cont*/, Server_Room *room)
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
			return RespChatFlood;
	}
	msg.replace(QChar('\n'), QChar(' '));
	
	room->say(this, msg);
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdCreateGame(const Command_CreateGame &cmd, CommandContainer * /*cont*/, Server_Room *room)
{
	if (authState == PasswordWrong)
		return RespLoginNeeded;

	if (server->getMaxGamesPerUser() > 0)
		if (room->getGamesCreatedByUser(userInfo->getName()) >= server->getMaxGamesPerUser())
			return RespContextError;
	
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
	
	sendProtocolItem(new Event_GameJoined(game->getGameId(), game->getDescription(), creator->getPlayerId(), creator->getPlayerId(), false, game->getSpectatorsCanTalk(), game->getSpectatorsSeeEverything(), false));
	sendProtocolItem(GameEventContainer::makeNew(new Event_GameStateChanged(game->getGameStarted(), game->getActivePlayer(), game->getActivePhase(), game->getGameState(creator)), game->getGameId()));
	
	game->gameMutex.unlock();
	
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdJoinGame(const Command_JoinGame &cmd, CommandContainer * /*cont*/, Server_Room *room)
{
	if (authState == PasswordWrong)
		return RespLoginNeeded;
	
	QMutexLocker gameListLocker(&gameListMutex);
	
	if (games.contains(cmd.game_id()))
		return RespContextError;
	
	Server_Game *g = room->getGames().value(cmd.game_id());
	if (!g)
		return RespNameNotFound;
	
	QMutexLocker locker(&g->gameMutex);
	
	ResponseCode result = g->checkJoin(userInfo, QString::fromStdString(cmd.password()), cmd.spectator(), cmd.override_restrictions());
	if (result == RespOk) {
		Server_Player *player = g->addPlayer(this, cmd.spectator());
		games.insert(cmd.game_id(), QPair<Server_Game *, Server_Player *>(g, player));
		enqueueProtocolItem(new Event_GameJoined(cmd.game_id(), g->getDescription(), g->getHostId(), player->getPlayerId(), cmd.spectator(), g->getSpectatorsCanTalk(), g->getSpectatorsSeeEverything(), false));
		enqueueProtocolItem(GameEventContainer::makeNew(new Event_GameStateChanged(g->getGameStarted(), g->getActivePlayer(), g->getActivePhase(), g->getGameState(player)), cmd.game_id()));
	}
	return result;
}

ResponseCode Server_ProtocolHandler::cmdLeaveGame(const Command_LeaveGame & /*cmd*/, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player, BlaContainer *bla)
{
	game->removePlayer(player);
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdKickFromGame(const Command_KickFromGame &cmd, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player, BlaContainer *bla)
{
	if ((game->getHostId() != player->getPlayerId()) && !(userInfo->getUserLevel() & ServerInfo_User::IsModerator))
		return RespFunctionNotAllowed;
	
	if (!game->kickPlayer(cmd.player_id()))
		return RespNameNotFound;
	
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdDeckSelect(const Command_DeckSelect &cmd, CommandContainer *cont, Server_Game *game, Server_Player *player, BlaContainer *bla)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	DeckList *deck;
	if (cmd.has_deck_id()) {
		try {
			deck = getDeckFromDatabase(cmd.deck_id());
		} catch(ResponseCode r) {
			return r;
		}
	} else
		deck = new DeckList(QString::fromStdString(cmd.deck()));

	player->setDeck(deck);
	
	game->sendGameEvent(new Event_PlayerPropertiesChanged(player->getPlayerId(), player->getProperties()), new Context_DeckSelect(deck->getDeckHash()));

	bla->setResponse(new Response_DeckDownload(cont->cmd_id(), RespOk, new DeckList(deck)));
	return RespNothing;
}

ResponseCode Server_ProtocolHandler::cmdSetSideboardPlan(const Command_SetSideboardPlan &cmd, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player, BlaContainer *bla)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	if (player->getReadyStart())
		return RespContextError;
	
	DeckList *deck = player->getDeck();
	if (!deck)
		return RespContextError;
	
	QList<MoveCardToZone *> sideboardPlan;
	for (int i = 0; i < cmd.move_list_size(); ++i) {
		const MoveCard_ToZone &temp = cmd.move_list(i);
		sideboardPlan.append(new MoveCardToZone(QString::fromStdString(temp.card_name()), QString::fromStdString(temp.start_zone()), QString::fromStdString(temp.target_zone())));
	}
	deck->setCurrentSideboardPlan(sideboardPlan);
	for (int i = 0; i < sideboardPlan.size(); ++i)
		delete sideboardPlan[i];
	// TEMPORARY HACK
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdConcede(const Command_Concede & /*cmd*/, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player, BlaContainer *bla)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
	
	player->setConceded(true);
	game->removeArrowsToPlayer(player);
	player->clearZones();
	game->sendGameEvent(new Event_PlayerPropertiesChanged(player->getPlayerId(), player->getProperties()), new Context_Concede);
	game->stopGameIfFinished();
	if (game->getGameStarted() && (game->getActivePlayer() == player->getPlayerId()))
		game->nextTurn();
	
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdReadyStart(const Command_ReadyStart &cmd, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player, BlaContainer *bla)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!player->getDeck() || game->getGameStarted())
		return RespContextError;

	if (player->getReadyStart() == cmd.ready())
		return RespContextError;
	
	player->setReadyStart(cmd.ready());
	game->sendGameEvent(new Event_PlayerPropertiesChanged(player->getPlayerId(), player->getProperties()), new Context_ReadyStart);
	game->startGameIfReady();
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdGameSay(const Command_GameSay &cmd, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player, BlaContainer *bla)
{
	if (player->getSpectator() && !game->getSpectatorsCanTalk() && !(userInfo->getUserLevel() & ServerInfo_User::IsModerator))
		return RespFunctionNotAllowed;
	
	game->sendGameEvent(new Event_Say(player->getPlayerId(), QString::fromStdString(cmd.message())));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdShuffle(const Command_Shuffle & /*cmd*/, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player, BlaContainer *bla)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
		
	player->getZones().value("deck")->shuffle();
	game->sendGameEvent(new Event_Shuffle(player->getPlayerId()));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdMulligan(const Command_Mulligan & /*cmd*/, CommandContainer *cont, Server_Game *game, Server_Player *player, BlaContainer *bla)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
	
	Server_CardZone *hand = player->getZones().value("hand");
	int number = (hand->cards.size() <= 1) ? player->getInitialCards() : hand->cards.size() - 1;
		
	Server_CardZone *deck = player->getZones().value("deck");
	while (!hand->cards.isEmpty()) {
		CardToMove *cardToMove = new CardToMove;
		cardToMove->set_card_id(hand->cards.first()->getId());
		player->moveCard(bla, hand, QList<const CardToMove *>() << cardToMove, deck, 0, 0, false);
		delete cardToMove;
	}

	deck->shuffle();
	bla->enqueueGameEventPrivate(new Event_Shuffle(player->getPlayerId()), game->getGameId());
	bla->enqueueGameEventOmniscient(new Event_Shuffle(player->getPlayerId()), game->getGameId());
	bla->enqueueGameEventPublic(new Event_Shuffle(player->getPlayerId()), game->getGameId());

	player->drawCards(bla, number);
	
	if (number == player->getInitialCards())
		number = -1;
	bla->getGameEventQueuePrivate()->setContext(new Context_Mulligan(number));
	bla->getGameEventQueuePublic()->setContext(new Context_Mulligan(number));
	bla->getGameEventQueueOmniscient()->setContext(new Context_Mulligan(number));

	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdRollDie(const Command_RollDie &cmd, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player, BlaContainer *bla)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	if (player->getConceded())
		return RespContextError;
	
	game->sendGameEvent(new Event_RollDie(player->getPlayerId(), cmd.sides(), rng->getNumber(1, cmd.sides())));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdDrawCards(const Command_DrawCards &cmd, CommandContainer *cont, Server_Game *game, Server_Player *player, BlaContainer *bla)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
		
	return player->drawCards(bla, cmd.number());
}

ResponseCode Server_ProtocolHandler::cmdUndoDraw(const Command_UndoDraw & /*cmd*/, CommandContainer *cont, Server_Game *game, Server_Player *player, BlaContainer *bla)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
		
	return player->undoDraw(bla);
}

ResponseCode Server_ProtocolHandler::cmdMoveCard(const Command_MoveCard &cmd, CommandContainer *cont, Server_Game *game, Server_Player *player, BlaContainer *bla)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
	
	QList<const CardToMove *> cardsToMove;
	for (int i = 0; i < cmd.cards_to_move().card_size(); ++i)
		cardsToMove.append(&cmd.cards_to_move().card(i));
	
	return player->moveCard(bla, QString::fromStdString(cmd.start_zone()), cardsToMove, cmd.target_player_id(), QString::fromStdString(cmd.target_zone()), cmd.x(), cmd.y());
}

ResponseCode Server_ProtocolHandler::cmdFlipCard(const Command_FlipCard &cmd, CommandContainer *cont, Server_Game *game, Server_Player *player, BlaContainer *bla)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
	
	Server_CardZone *zone = player->getZones().value(QString::fromStdString(cmd.zone()));
	if (!zone)
		return RespNameNotFound;
	if (!zone->hasCoords())
		return RespContextError;
	
	Server_Card *card = zone->getCard(cmd.card_id());
	if (!card)
		return RespNameNotFound;
	
	const bool faceDown = cmd.face_down();
	if (faceDown == card->getFaceDown())
		return RespContextError;
	
	card->setFaceDown(faceDown);
	bla->enqueueGameEventPrivate(new Event_FlipCard(player->getPlayerId(), zone->getName(), card->getId(), card->getName(), faceDown), game->getGameId());
	bla->enqueueGameEventPublic(new Event_FlipCard(player->getPlayerId(), zone->getName(), card->getId(), card->getName(), faceDown), game->getGameId());
	
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdAttachCard(const Command_AttachCard &cmd, CommandContainer *cont, Server_Game *game, Server_Player *player, BlaContainer *bla)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
		
	Server_CardZone *startzone = player->getZones().value(QString::fromStdString(cmd.start_zone()));
	if (!startzone)
		return RespNameNotFound;
	
	Server_Card *card = startzone->getCard(cmd.card_id());
	if (!card)
		return RespNameNotFound;

	int playerId = cmd.target_player_id();
	Server_Player *targetPlayer = 0;
	Server_CardZone *targetzone = 0;
	Server_Card *targetCard = 0;
	
	if (playerId != -1) {
		targetPlayer = game->getPlayer(cmd.target_player_id());
		if (!targetPlayer)
			return RespNameNotFound;
	} else if (!card->getParentCard())
		return RespContextError;
	if (targetPlayer)
		targetzone = targetPlayer->getZones().value(QString::fromStdString(cmd.target_zone()));
	if (targetzone) {
		// This is currently enough to make sure cards don't get attached to a card that is not on the table.
		// Possibly a flag will have to be introduced for this sometime.
		if (!targetzone->hasCoords())
			return RespContextError;
		targetCard = targetzone->getCard(cmd.target_card_id());
		if (targetCard)
			if (targetCard->getParentCard())
				return RespContextError;
	}
	if (!startzone->hasCoords())
		return RespContextError;
	
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
			bla->enqueueGameEventPrivate(new Event_DeleteArrow(p->getPlayerId(), toDelete[i]->getId()), game->getGameId());
			bla->enqueueGameEventPublic(new Event_DeleteArrow(p->getPlayerId(), toDelete[i]->getId()), game->getGameId());
			p->deleteArrow(toDelete[i]->getId());
		}
	}

	if (targetCard) {
		// Unattach all cards attached to the card being attached.
		// Make a copy of the list because its contents change during the loop otherwise.
		QList<Server_Card *> attachedList = card->getAttachedCards();
		for (int i = 0; i < attachedList.size(); ++i)
			attachedList[i]->getZone()->getPlayer()->unattachCard(bla, attachedList[i]);
		
		if (targetzone->isColumnStacked(targetCard->getX(), targetCard->getY())) {
			CardToMove *cardToMove = new CardToMove;
			cardToMove->set_card_id(targetCard->getId());
			targetPlayer->moveCard(bla, targetzone, QList<const CardToMove *>() << cardToMove, targetzone, targetzone->getFreeGridColumn(-2, targetCard->getY(), targetCard->getName()), targetCard->getY(), targetCard->getFaceDown());
			delete cardToMove;
		}
		
		card->setParentCard(targetCard);
		card->setCoords(-1, card->getY());
		bla->enqueueGameEventPrivate(new Event_AttachCard(player->getPlayerId(), startzone->getName(), card->getId(), targetPlayer->getPlayerId(), targetzone->getName(), targetCard->getId()), game->getGameId());
		bla->enqueueGameEventPublic(new Event_AttachCard(player->getPlayerId(), startzone->getName(), card->getId(), targetPlayer->getPlayerId(), targetzone->getName(), targetCard->getId()), game->getGameId());
		startzone->fixFreeSpaces(bla);
	} else
		player->unattachCard(bla, card);
	
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdCreateToken(const Command_CreateToken &cmd, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player, BlaContainer *bla)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
		
	Server_CardZone *zone = player->getZones().value(QString::fromStdString(cmd.zone()));
	if (!zone)
		return RespNameNotFound;

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
	game->sendGameEvent(new Event_CreateToken(player->getPlayerId(), zone->getName(), card->getId(), card->getName(), card->getColor(), card->getPT(), card->getAnnotation(), card->getDestroyOnZoneChange(), x, y));
	
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdCreateArrow(const Command_CreateArrow &cmd, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player, BlaContainer *bla)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
	
	Server_Player *startPlayer = game->getPlayer(cmd.start_player_id());
	Server_Player *targetPlayer = game->getPlayer(cmd.target_player_id());
	if (!startPlayer || !targetPlayer)
		return RespNameNotFound;
	QString startZoneName = QString::fromStdString(cmd.start_zone());
	Server_CardZone *startZone = startPlayer->getZones().value(startZoneName);
	QString targetZoneName = QString::fromStdString(cmd.target_zone());
	bool playerTarget = targetZoneName.isEmpty();
	Server_CardZone *targetZone = 0;
	if (!playerTarget)
		targetZone = targetPlayer->getZones().value(targetZoneName);
	if (!startZone || (!targetZone && !playerTarget))
		return RespNameNotFound;
	if (startZone->getType() != PublicZone)
		return RespContextError;
	Server_Card *startCard = startZone->getCard(cmd.start_card_id());
	if (!startCard)
		return RespNameNotFound;
	Server_Card *targetCard = 0;
	if (!playerTarget) {
		if (targetZone->getType() != PublicZone)
			return RespContextError;
		targetCard = targetZone->getCard(cmd.target_card_id());
	}
	
	Server_ArrowTarget *targetItem;
	if (playerTarget)
		targetItem = targetPlayer;
	else
		targetItem = targetCard;
	if (!targetItem)
		return RespNameNotFound;

	QMapIterator<int, Server_Arrow *> arrowIterator(player->getArrows());
	while (arrowIterator.hasNext()) {
		Server_Arrow *temp = arrowIterator.next().value();
		if ((temp->getStartCard() == startCard) && (temp->getTargetItem() == targetItem))
			return RespContextError;
	}
	
	Server_Arrow *arrow = new Server_Arrow(player->newArrowId(), startCard, targetItem, cmd.arrow_color());
	player->addArrow(arrow);
	game->sendGameEvent(new Event_CreateArrows(player->getPlayerId(), QList<ServerInfo_Arrow *>() << new ServerInfo_Arrow(
		arrow->getId(),
		startPlayer->getPlayerId(),
		startZoneName,
		startCard->getId(),
		targetPlayer->getPlayerId(),
		targetZoneName,
		cmd.target_card_id(),
		cmd.arrow_color()
	)));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdDeleteArrow(const Command_DeleteArrow &cmd, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player, BlaContainer *bla)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
	
	if (!player->deleteArrow(cmd.arrow_id()))
		return RespNameNotFound;
	
	game->sendGameEvent(new Event_DeleteArrow(player->getPlayerId(), cmd.arrow_id()));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdSetCardAttr(const Command_SetCardAttr &cmd, CommandContainer *cont, Server_Game *game, Server_Player *player, BlaContainer *bla)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
	
	return player->setCardAttrHelper(bla, QString::fromStdString(cmd.zone()), cmd.card_id(), QString::fromStdString(cmd.attr_name()), QString::fromStdString(cmd.attr_value()));
}

ResponseCode Server_ProtocolHandler::cmdSetCardCounter(const Command_SetCardCounter &cmd, CommandContainer *cont, Server_Game *game, Server_Player *player, BlaContainer *bla)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
	
	Server_CardZone *zone = player->getZones().value(QString::fromStdString(cmd.zone()));
	if (!zone)
		return RespNameNotFound;
	if (!zone->hasCoords())
		return RespContextError;

	Server_Card *card = zone->getCard(cmd.card_id());
	if (!card)
		return RespNameNotFound;
	
	card->setCounter(cmd.counter_id(), cmd.counter_value());
	
	bla->enqueueGameEventPrivate(new Event_SetCardCounter(player->getPlayerId(), zone->getName(), card->getId(), cmd.counter_id(), cmd.counter_value()), game->getGameId());
	bla->enqueueGameEventPublic(new Event_SetCardCounter(player->getPlayerId(), zone->getName(), card->getId(), cmd.counter_id(), cmd.counter_value()), game->getGameId());
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdIncCardCounter(const Command_IncCardCounter &cmd, CommandContainer *cont, Server_Game *game, Server_Player *player, BlaContainer *bla)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
	
	Server_CardZone *zone = player->getZones().value(QString::fromStdString(cmd.zone()));
	if (!zone)
		return RespNameNotFound;
	if (!zone->hasCoords())
		return RespContextError;

	Server_Card *card = zone->getCard(cmd.card_id());
	if (!card)
		return RespNameNotFound;
	
	int newValue = card->getCounter(cmd.counter_id()) + cmd.counter_delta();
	card->setCounter(cmd.counter_id(), newValue);
	
	bla->enqueueGameEventPrivate(new Event_SetCardCounter(player->getPlayerId(), zone->getName(), card->getId(), cmd.counter_id(), newValue), game->getGameId());
	bla->enqueueGameEventPublic(new Event_SetCardCounter(player->getPlayerId(), zone->getName(), card->getId(), cmd.counter_id(), newValue), game->getGameId());
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdIncCounter(const Command_IncCounter &cmd, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player, BlaContainer *bla)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
	
	const QMap<int, Server_Counter *> counters = player->getCounters();
	Server_Counter *c = counters.value(cmd.counter_id(), 0);
	if (!c)
		return RespNameNotFound;
	
	c->setCount(c->getCount() + cmd.delta());
	game->sendGameEvent(new Event_SetCounter(player->getPlayerId(), c->getId(), c->getCount()));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdCreateCounter(const Command_CreateCounter &cmd, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player, BlaContainer *bla)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
	
	Server_Counter *c = new Server_Counter(player->newCounterId(), QString::fromStdString(cmd.counter_name()), cmd.counter_color(), cmd.radius(), cmd.value());
	player->addCounter(c);
	game->sendGameEvent(new Event_CreateCounters(player->getPlayerId(), QList<ServerInfo_Counter *>() << new ServerInfo_Counter(c->getId(), c->getName(), c->getColor(), c->getRadius(), c->getCount())));
	
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdSetCounter(const Command_SetCounter &cmd, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player, BlaContainer *bla)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
	
	Server_Counter *c = player->getCounters().value(cmd.counter_id(), 0);;
	if (!c)
		return RespNameNotFound;
	
	c->setCount(cmd.value());
	game->sendGameEvent(new Event_SetCounter(player->getPlayerId(), c->getId(), c->getCount()));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdDelCounter(const Command_DelCounter &cmd, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player, BlaContainer *bla)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
	
	if (!player->deleteCounter(cmd.counter_id()))
		return RespNameNotFound;
	game->sendGameEvent(new Event_DelCounter(player->getPlayerId(), cmd.counter_id()));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdNextTurn(const Command_NextTurn & /*cmd*/, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player, BlaContainer *bla)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
	
	game->nextTurn();
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdSetActivePhase(const Command_SetActivePhase &cmd, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player, BlaContainer *bla)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
	
	if (game->getActivePlayer() != player->getPlayerId())
		return RespContextError;
	game->setActivePhase(cmd.phase());
	
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdDumpZone(const Command_DumpZone &cmd, CommandContainer *cont, Server_Game *game, Server_Player *player, BlaContainer *bla)
{
	if (!game->getGameStarted())
		return RespGameNotStarted;
	
	Server_Player *otherPlayer = game->getPlayer(cmd.player_id());
	if (!otherPlayer)
		return RespNameNotFound;
	Server_CardZone *zone = otherPlayer->getZones().value(QString::fromStdString(cmd.zone_name()));
	if (!zone)
		return RespNameNotFound;
	if (!((zone->getType() == PublicZone) || (player == otherPlayer)))
		return RespContextError;
	
	int numberCards = cmd.number_cards();
	QList<ServerInfo_Card *> respCardList;
	for (int i = 0; (i < zone->cards.size()) && (i < numberCards || numberCards == -1); ++i) {
		Server_Card *card = zone->cards[i];
		QString displayedName = card->getFaceDown() ? QString() : card->getName();
		if (zone->getType() == HiddenZone)
			respCardList.append(new ServerInfo_Card(i, displayedName));
		else {
			QList<ServerInfo_CardCounter *> cardCounterList;
			QMapIterator<int, int> cardCounterIterator(card->getCounters());
			while (cardCounterIterator.hasNext()) {
				cardCounterIterator.next();
				cardCounterList.append(new ServerInfo_CardCounter(cardCounterIterator.key(), cardCounterIterator.value()));
			}

			int attachPlayerId = -1;
			QString attachZone;
			int attachCardId = -1;
			if (card->getParentCard()) {
				attachPlayerId = card->getParentCard()->getZone()->getPlayer()->getPlayerId();
				attachZone = card->getParentCard()->getZone()->getName();
				attachCardId = card->getParentCard()->getId();
			}
			
			respCardList.append(new ServerInfo_Card(card->getId(), displayedName, card->getX(), card->getY(), card->getFaceDown(), card->getTapped(), card->getAttacking(), card->getColor(), card->getPT(), card->getAnnotation(), card->getDestroyOnZoneChange(), card->getDoesntUntap(), cardCounterList, attachPlayerId, attachZone, attachCardId));
		}
	}
	if (zone->getType() == HiddenZone) {
		zone->setCardsBeingLookedAt(numberCards);
		game->sendGameEvent(new Event_DumpZone(player->getPlayerId(), otherPlayer->getPlayerId(), zone->getName(), numberCards));
	}
	bla->setResponse(new Response_DumpZone(cont->cmd_id(), RespOk, new ServerInfo_Zone(zone->getName(), zone->getType(), zone->hasCoords(), numberCards < zone->cards.size() ? zone->cards.size() : numberCards, respCardList)));
	return RespNothing;
}

ResponseCode Server_ProtocolHandler::cmdStopDumpZone(const Command_StopDumpZone &cmd, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player, BlaContainer *bla)
{
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
	
	Server_Player *otherPlayer = game->getPlayer(cmd.player_id());
	if (!otherPlayer)
		return RespNameNotFound;
	Server_CardZone *zone = otherPlayer->getZones().value(QString::fromStdString(cmd.zone_name()));
	if (!zone)
		return RespNameNotFound;
	
	if (zone->getType() == HiddenZone) {
		zone->setCardsBeingLookedAt(0);
		game->sendGameEvent(new Event_StopDumpZone(player->getPlayerId(), cmd.player_id(), zone->getName()));
	}
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdRevealCards(const Command_RevealCards &cmd, CommandContainer *cont, Server_Game *game, Server_Player *player, BlaContainer *bla)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
	
	Server_Player *otherPlayer = 0;
	if (cmd.player_id() != -1) {
		otherPlayer = game->getPlayer(cmd.player_id());
		if (!otherPlayer)
			return RespNameNotFound;
	}
	Server_CardZone *zone = player->getZones().value(QString::fromStdString(cmd.zone_name()));
	if (!zone)
		return RespNameNotFound;
	
	QList<Server_Card *> cardsToReveal;
	if (cmd.card_id() == -1)
		cardsToReveal = zone->cards;
	else if (cmd.card_id() == -2) {
		if (zone->cards.isEmpty())
			return RespContextError;
		cardsToReveal.append(zone->cards.at(rng->getNumber(0, zone->cards.size() - 1)));
	} else {
		Server_Card *card = zone->getCard(cmd.card_id());
		if (!card)
			return RespNameNotFound;
		cardsToReveal.append(card);
	}
	
	QList<ServerInfo_Card *> respCardListPrivate, respCardListOmniscient;
	for (int i = 0; i < cardsToReveal.size(); ++i) {
		Server_Card *card = cardsToReveal[i];

		QList<ServerInfo_CardCounter *> cardCounterListPrivate, cardCounterListOmniscient;
		QMapIterator<int, int> cardCounterIterator(card->getCounters());
		while (cardCounterIterator.hasNext()) {
			cardCounterIterator.next();
			cardCounterListPrivate.append(new ServerInfo_CardCounter(cardCounterIterator.key(), cardCounterIterator.value()));
			cardCounterListOmniscient.append(new ServerInfo_CardCounter(cardCounterIterator.key(), cardCounterIterator.value()));
		}
		
		int attachPlayerId = -1;
		QString attachZone;
		int attachCardId = -1;
		if (card->getParentCard()) {
			attachPlayerId = card->getParentCard()->getZone()->getPlayer()->getPlayerId();
			attachZone = card->getParentCard()->getZone()->getName();
			attachCardId = card->getParentCard()->getId();
		}
		
		if (cmd.player_id() != -1)
			respCardListPrivate.append(new ServerInfo_Card(card->getId(), card->getName(), card->getX(), card->getY(), card->getFaceDown(), card->getTapped(), card->getAttacking(), card->getColor(), card->getPT(), card->getAnnotation(), card->getDestroyOnZoneChange(), card->getDoesntUntap(), cardCounterListPrivate, attachPlayerId, attachZone, attachCardId));
		respCardListOmniscient.append(new ServerInfo_Card(card->getId(), card->getName(), card->getX(), card->getY(), card->getFaceDown(), card->getTapped(), card->getAttacking(), card->getColor(), card->getPT(), card->getAnnotation(), card->getDestroyOnZoneChange(), card->getDoesntUntap(), cardCounterListOmniscient, attachPlayerId, attachZone, attachCardId));
	}
	
	if (cmd.player_id() == -1)
		bla->enqueueGameEventPublic(new Event_RevealCards(player->getPlayerId(), zone->getName(), cmd.card_id(), -1, respCardListOmniscient), game->getGameId());
	else {
		bla->enqueueGameEventPublic(new Event_RevealCards(player->getPlayerId(), zone->getName(), cmd.card_id(), otherPlayer->getPlayerId()), game->getGameId());
		bla->enqueueGameEventPrivate(new Event_RevealCards(player->getPlayerId(), zone->getName(), cmd.card_id(), otherPlayer->getPlayerId(), respCardListPrivate), game->getGameId(), otherPlayer->getPlayerId());
		bla->enqueueGameEventOmniscient(new Event_RevealCards(player->getPlayerId(), zone->getName(), cmd.card_id(), otherPlayer->getPlayerId(), respCardListOmniscient), game->getGameId());
	}
	
	return RespOk;
}
