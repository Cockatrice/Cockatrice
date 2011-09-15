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

Server_ProtocolHandler::Server_ProtocolHandler(Server *_server, QObject *parent)
	: QObject(parent), server(_server), authState(PasswordWrong), acceptsUserListChanges(false), acceptsRoomListChanges(false), userInfo(0), timeRunning(0), lastDataReceived(0), gameListMutex(QMutex::Recursive)
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

ResponseCode Server_ProtocolHandler::processCommandHelper(Command *command, CommandContainer *cont)
{
	RoomCommand *roomCommand = qobject_cast<RoomCommand *>(command);
	if (roomCommand) {
		qDebug() << "received RoomCommand: roomId =" << roomCommand->getRoomId();
		if (authState == PasswordWrong)
			return RespLoginNeeded;
	
		Server_Room *room = rooms.value(roomCommand->getRoomId(), 0);
		if (!room)
			return RespNotInRoom;
		
		QMutexLocker locker(&room->roomMutex);
		
		switch (command->getItemId()) {
			case ItemId_Command_LeaveRoom: return cmdLeaveRoom(static_cast<Command_LeaveRoom *>(command), cont, room);
			case ItemId_Command_RoomSay: return cmdRoomSay(static_cast<Command_RoomSay *>(command), cont, room);
			case ItemId_Command_CreateGame: return cmdCreateGame(static_cast<Command_CreateGame *>(command), cont, room);
			case ItemId_Command_JoinGame: return cmdJoinGame(static_cast<Command_JoinGame *>(command), cont, room);
			default: return RespInvalidCommand;
		}
	}
	GameCommand *gameCommand = qobject_cast<GameCommand *>(command);
	if (gameCommand) {
		qDebug() << "received GameCommand: game =" << gameCommand->getGameId();
		if (authState == PasswordWrong)
			return RespLoginNeeded;
		
		gameListMutex.lock();
		if (!games.contains(gameCommand->getGameId())) {
			qDebug() << "invalid game";
			return RespNotInRoom;
		}
		QPair<Server_Game *, Server_Player *> gamePair = games.value(gameCommand->getGameId());
		Server_Game *game = gamePair.first;
		Server_Player *player = gamePair.second;
		
		QMutexLocker locker(&game->gameMutex);
		gameListMutex.unlock();
		
		switch (command->getItemId()) {
			case ItemId_Command_DeckSelect: return cmdDeckSelect(static_cast<Command_DeckSelect *>(command), cont, game, player);
			case ItemId_Command_SetSideboardPlan: return cmdSetSideboardPlan(static_cast<Command_SetSideboardPlan *>(command), cont, game, player);
			case ItemId_Command_LeaveGame: return cmdLeaveGame(static_cast<Command_LeaveGame *>(command), cont, game, player);
			case ItemId_Command_KickFromGame: return cmdKickFromGame(static_cast<Command_KickFromGame *>(command), cont, game, player);
			case ItemId_Command_ReadyStart: return cmdReadyStart(static_cast<Command_ReadyStart *>(command), cont, game, player);
			case ItemId_Command_Concede: return cmdConcede(static_cast<Command_Concede *>(command), cont, game, player);
			case ItemId_Command_Say: return cmdSay(static_cast<Command_Say *>(command), cont, game, player);
			case ItemId_Command_Shuffle: return cmdShuffle(static_cast<Command_Shuffle *>(command), cont, game, player);
			case ItemId_Command_Mulligan: return cmdMulligan(static_cast<Command_Mulligan *>(command), cont, game, player);
			case ItemId_Command_RollDie: return cmdRollDie(static_cast<Command_RollDie *>(command), cont, game, player);
			case ItemId_Command_DrawCards: return cmdDrawCards(static_cast<Command_DrawCards *>(command), cont, game, player);
			case ItemId_Command_UndoDraw: return cmdUndoDraw(static_cast<Command_UndoDraw *>(command), cont, game, player);
			case ItemId_Command_MoveCard: return cmdMoveCard(static_cast<Command_MoveCard *>(command), cont, game, player);
			case ItemId_Command_FlipCard: return cmdFlipCard(static_cast<Command_FlipCard *>(command), cont, game, player);
			case ItemId_Command_AttachCard: return cmdAttachCard(static_cast<Command_AttachCard *>(command), cont, game, player);
			case ItemId_Command_CreateToken: return cmdCreateToken(static_cast<Command_CreateToken *>(command), cont, game, player);
			case ItemId_Command_CreateArrow: return cmdCreateArrow(static_cast<Command_CreateArrow *>(command), cont, game, player);
			case ItemId_Command_DeleteArrow: return cmdDeleteArrow(static_cast<Command_DeleteArrow *>(command), cont, game, player);
			case ItemId_Command_SetCardAttr: return cmdSetCardAttr(static_cast<Command_SetCardAttr *>(command), cont, game, player);
			case ItemId_Command_SetCardCounter: return cmdSetCardCounter(static_cast<Command_SetCardCounter *>(command), cont, game, player);
			case ItemId_Command_IncCardCounter: return cmdIncCardCounter(static_cast<Command_IncCardCounter *>(command), cont, game, player);
			case ItemId_Command_IncCounter: return cmdIncCounter(static_cast<Command_IncCounter *>(command), cont, game, player);
			case ItemId_Command_CreateCounter: return cmdCreateCounter(static_cast<Command_CreateCounter *>(command), cont, game, player);
			case ItemId_Command_SetCounter: return cmdSetCounter(static_cast<Command_SetCounter *>(command), cont, game, player);
			case ItemId_Command_DelCounter: return cmdDelCounter(static_cast<Command_DelCounter *>(command), cont, game, player);
			case ItemId_Command_NextTurn: return cmdNextTurn(static_cast<Command_NextTurn *>(command), cont, game, player);
			case ItemId_Command_SetActivePhase: return cmdSetActivePhase(static_cast<Command_SetActivePhase *>(command), cont, game, player);
			case ItemId_Command_DumpZone: return cmdDumpZone(static_cast<Command_DumpZone *>(command), cont, game, player);
			case ItemId_Command_StopDumpZone: return cmdStopDumpZone(static_cast<Command_StopDumpZone *>(command), cont, game, player);
			case ItemId_Command_RevealCards: return cmdRevealCards(static_cast<Command_RevealCards *>(command), cont, game, player);
			default: return RespInvalidCommand;
		}
	}
	ModeratorCommand *moderatorCommand = qobject_cast<ModeratorCommand *>(command);
	if (moderatorCommand) {
		qDebug() << "received ModeratorCommand";
		if (!(userInfo->getUserLevel() & ServerInfo_User::IsModerator))
			return RespLoginNeeded;
		
		switch (command->getItemId()) {
			case ItemId_Command_BanFromServer: return cmdBanFromServer(static_cast<Command_BanFromServer *>(command), cont);
			default: return RespInvalidCommand;
		}
	}
	AdminCommand *adminCommand = qobject_cast<AdminCommand *>(command);
	if (adminCommand) {
		qDebug() << "received AdminCommand";
		if (!(userInfo->getUserLevel() & ServerInfo_User::IsAdmin))
			return RespLoginNeeded;
		
		switch (command->getItemId()) {
			case ItemId_Command_ShutdownServer: return cmdShutdownServer(static_cast<Command_ShutdownServer *>(command), cont);
			case ItemId_Command_UpdateServerMessage: return cmdUpdateServerMessage(static_cast<Command_UpdateServerMessage *>(command), cont);
			default: return RespInvalidCommand;
		}
	}
	switch (command->getItemId()) {
		case ItemId_Command_Ping: return cmdPing(static_cast<Command_Ping *>(command), cont);
		case ItemId_Command_Login: return cmdLogin(static_cast<Command_Login *>(command), cont);
		case ItemId_Command_Message: return cmdMessage(static_cast<Command_Message *>(command), cont);
		case ItemId_Command_AddToList: return cmdAddToList(static_cast<Command_AddToList *>(command), cont);
		case ItemId_Command_RemoveFromList: return cmdRemoveFromList(static_cast<Command_RemoveFromList *>(command), cont);
		case ItemId_Command_DeckList: return cmdDeckList(static_cast<Command_DeckList *>(command), cont);
		case ItemId_Command_DeckNewDir: return cmdDeckNewDir(static_cast<Command_DeckNewDir *>(command), cont);
		case ItemId_Command_DeckDelDir: return cmdDeckDelDir(static_cast<Command_DeckDelDir *>(command), cont);
		case ItemId_Command_DeckDel: return cmdDeckDel(static_cast<Command_DeckDel *>(command), cont);
		case ItemId_Command_DeckUpload: return cmdDeckUpload(static_cast<Command_DeckUpload *>(command), cont);
		case ItemId_Command_DeckDownload: return cmdDeckDownload(static_cast<Command_DeckDownload *>(command), cont);
		case ItemId_Command_GetGamesOfUser: return cmdGetGamesOfUser(static_cast<Command_GetGamesOfUser *>(command), cont);
		case ItemId_Command_GetUserInfo: return cmdGetUserInfo(static_cast<Command_GetUserInfo *>(command), cont);
		case ItemId_Command_ListRooms: return cmdListRooms(static_cast<Command_ListRooms *>(command), cont);
		case ItemId_Command_JoinRoom: return cmdJoinRoom(static_cast<Command_JoinRoom *>(command), cont);
		case ItemId_Command_ListUsers: return cmdListUsers(static_cast<Command_ListUsers *>(command), cont);
		default: return RespInvalidCommand;
	}
}

void Server_ProtocolHandler::processCommandContainer(CommandContainer *cont)
{
	lastDataReceived = timeRunning;
	
	const QList<Command *> &cmdList = cont->getCommandList();
	ResponseCode finalResponseCode = RespOk;
	for (int i = 0; i < cmdList.size(); ++i) {
		ResponseCode resp = processCommandHelper(cmdList[i], cont);
		if ((resp != RespOk) && (resp != RespNothing))
			finalResponseCode = resp;
	}
	
	ProtocolResponse *pr = cont->getResponse();
	if (!pr)
		pr = new ProtocolResponse(cont->getCmdId(), finalResponseCode);
	
	gameListMutex.lock();
	GameEventContainer *gQPublic = cont->getGameEventQueuePublic();
	if (gQPublic) {
		QPair<Server_Game *, Server_Player *> gamePlayerPair = games.value(gQPublic->getGameId());
		if (gamePlayerPair.first) {
			GameEventContainer *gQPrivate = cont->getGameEventQueuePrivate();
			GameEventContainer *gQOmniscient = cont->getGameEventQueueOmniscient();
			if (gQPrivate) {
				int privatePlayerId = cont->getPrivatePlayerId();
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
	
	const QList<ProtocolItem *> &iQ = cont->getItemQueue();
	for (int i = 0; i < iQ.size(); ++i)
		sendProtocolItem(iQ[i]);
	
	sendProtocolItem(pr);
	
	while (!itemQueue.isEmpty())
		sendProtocolItem(itemQueue.takeFirst());

	if (cont->getReceiverMayDelete())
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

ResponseCode Server_ProtocolHandler::cmdPing(Command_Ping * /*cmd*/, CommandContainer * /*cont*/)
{
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdLogin(Command_Login *cmd, CommandContainer *cont)
{
	QString userName = cmd->getUsername().simplified();
	if (userName.isEmpty() || (userInfo != 0))
		return RespContextError;
	authState = server->loginUser(this, userName, cmd->getPassword());
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
					
					enqueueProtocolItem(new Event_GameJoined(game->getGameId(), game->getDescription(), gamePlayers[j]->getPlayerId(), gamePlayers[j]->getSpectator(), game->getSpectatorsCanTalk(), game->getSpectatorsSeeEverything(), true));
					enqueueProtocolItem(GameEventContainer::makeNew(new Event_GameStateChanged(game->getGameStarted(), game->getActivePlayer(), game->getActivePhase(), game->getGameState(gamePlayers[j])), game->getGameId()));
					
					break;
				}
		}
		room->roomMutex.unlock();
	}
	server->serverMutex.unlock();
	
	ProtocolResponse *resp = new Response_Login(cont->getCmdId(), RespOk, new ServerInfo_User(userInfo, true), _buddyList, _ignoreList);
	if (getCompressionSupport())
		resp->setCompressed(true);
	cont->setResponse(resp);
	return RespNothing;
}

ResponseCode Server_ProtocolHandler::cmdMessage(Command_Message *cmd, CommandContainer *cont)
{
	if (authState == PasswordWrong)
		return RespLoginNeeded;
	
	QString receiver = cmd->getUserName();
	Server_ProtocolHandler *userHandler = server->getUsers().value(receiver);
	qDebug() << "cmdMessage: recv=" << receiver << (userHandler == 0 ? "not found" : "found");
	if (!userHandler)
		return RespNameNotFound;
	if (userHandler->getIgnoreList().contains(userInfo->getName()))
		return RespInIgnoreList;
	
	cont->enqueueItem(new Event_Message(userInfo->getName(), receiver, cmd->getText()));
	userHandler->sendProtocolItem(new Event_Message(userInfo->getName(), receiver, cmd->getText()));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdGetGamesOfUser(Command_GetGamesOfUser *cmd, CommandContainer *cont)
{
	if (authState == PasswordWrong)
		return RespLoginNeeded;
	
	server->serverMutex.lock();
	if (!server->getUsers().contains(cmd->getUserName()))
		return RespNameNotFound;
	
	QList<ServerInfo_Room *> roomList;
	QList<ServerInfo_Game *> gameList;
	QMapIterator<int, Server_Room *> roomIterator(server->getRooms());
	while (roomIterator.hasNext()) {
		Server_Room *room = roomIterator.next().value();
		room->roomMutex.lock();
		roomList.append(room->getInfo(false, true));
		gameList << room->getGamesOfUser(cmd->getUserName());
		room->roomMutex.unlock();
	}
	server->serverMutex.unlock();
	
	ProtocolResponse *resp = new Response_GetGamesOfUser(cont->getCmdId(), RespOk, roomList, gameList);
	if (getCompressionSupport())
		resp->setCompressed(true);
	cont->setResponse(resp);
	return RespNothing;
}

ResponseCode Server_ProtocolHandler::cmdGetUserInfo(Command_GetUserInfo *cmd, CommandContainer *cont)
{
	if (authState == PasswordWrong)
		return RespLoginNeeded;
	
	ServerInfo_User *result;
	if (cmd->getUserName().isEmpty())
		result = new ServerInfo_User(userInfo);
	else {
		Server_ProtocolHandler *handler = server->getUsers().value(cmd->getUserName());
		if (!handler)
			return RespNameNotFound;
		result = new ServerInfo_User(handler->getUserInfo());
	}
	
	cont->setResponse(new Response_GetUserInfo(cont->getCmdId(), RespOk, result));
	return RespNothing;
}

ResponseCode Server_ProtocolHandler::cmdListRooms(Command_ListRooms * /*cmd*/, CommandContainer *cont)
{
	if (authState == PasswordWrong)
		return RespLoginNeeded;
	
	QList<ServerInfo_Room *> eventRoomList;
	QMapIterator<int, Server_Room *> roomIterator(server->getRooms());
	while (roomIterator.hasNext())
		eventRoomList.append(roomIterator.next().value()->getInfo(false));
	cont->enqueueItem(new Event_ListRooms(eventRoomList));
	
	acceptsRoomListChanges = true;
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdJoinRoom(Command_JoinRoom *cmd, CommandContainer *cont)
{
	if (authState == PasswordWrong)
		return RespLoginNeeded;
	
	if (rooms.contains(cmd->getRoomId()))
		return RespContextError;
	
	Server_Room *r = server->getRooms().value(cmd->getRoomId(), 0);
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
	cont->setResponse(new Response_JoinRoom(cont->getCmdId(), RespOk, info));
	return RespNothing;
}

ResponseCode Server_ProtocolHandler::cmdLeaveRoom(Command_LeaveRoom * /*cmd*/, CommandContainer * /*cont*/, Server_Room *room)
{
	rooms.remove(room->getId());
	room->removeClient(this);
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdRoomSay(Command_RoomSay *cmd, CommandContainer * /*cont*/, Server_Room *room)
{
	QString msg = cmd->getMessage();
	
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

ResponseCode Server_ProtocolHandler::cmdListUsers(Command_ListUsers * /*cmd*/, CommandContainer *cont)
{
	if (authState == PasswordWrong)
		return RespLoginNeeded;
	
	QList<ServerInfo_User *> resultList;
	QMapIterator<QString, Server_ProtocolHandler *> userIterator = server->getUsers();
	while (userIterator.hasNext())
		resultList.append(new ServerInfo_User(userIterator.next().value()->getUserInfo(), false));
	
	acceptsUserListChanges = true;
	
	ProtocolResponse *resp = new Response_ListUsers(cont->getCmdId(), RespOk, resultList);
	if (getCompressionSupport())
		resp->setCompressed(true);
	cont->setResponse(resp);
	return RespNothing;
}

ResponseCode Server_ProtocolHandler::cmdCreateGame(Command_CreateGame *cmd, CommandContainer * /*cont*/, Server_Room *room)
{
	if (authState == PasswordWrong)
		return RespLoginNeeded;

	if (server->getMaxGamesPerUser() > 0)
		if (room->getGamesCreatedByUser(userInfo->getName()) >= server->getMaxGamesPerUser())
			return RespContextError;
	
	QList<int> gameTypes;
	QList<GameTypeId *> gameTypeList = cmd->getGameTypes();
	for (int i = 0; i < gameTypeList.size(); ++i)
		gameTypes.append(gameTypeList[i]->getData());
	
	QString description = cmd->getDescription();
	if (description.size() > 60)
		description = description.left(60);
	Server_Game *game = room->createGame(description, cmd->getPassword(), cmd->getMaxPlayers(), gameTypes, cmd->getOnlyBuddies(), cmd->getOnlyRegistered(), cmd->getSpectatorsAllowed(), cmd->getSpectatorsNeedPassword(), cmd->getSpectatorsCanTalk(), cmd->getSpectatorsSeeEverything(), this);
	
	Server_Player *creator = game->getPlayers().values().first();
	
	QMutexLocker gameListLocker(&gameListMutex);
	games.insert(game->getGameId(), QPair<Server_Game *, Server_Player *>(game, creator));
	
	sendProtocolItem(new Event_GameJoined(game->getGameId(), game->getDescription(), creator->getPlayerId(), false, game->getSpectatorsCanTalk(), game->getSpectatorsSeeEverything(), false));
	sendProtocolItem(GameEventContainer::makeNew(new Event_GameStateChanged(game->getGameStarted(), game->getActivePlayer(), game->getActivePhase(), game->getGameState(creator)), game->getGameId()));
	
	game->gameMutex.unlock();
	
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdJoinGame(Command_JoinGame *cmd, CommandContainer * /*cont*/, Server_Room *room)
{
	if (authState == PasswordWrong)
		return RespLoginNeeded;
	
	QMutexLocker gameListLocker(&gameListMutex);
	
	if (games.contains(cmd->getGameId()))
		return RespContextError;
	
	Server_Game *g = room->getGames().value(cmd->getGameId());
	if (!g)
		return RespNameNotFound;
	
	QMutexLocker locker(&g->gameMutex);
	
	ResponseCode result = g->checkJoin(userInfo, cmd->getPassword(), cmd->getSpectator());
	if (result == RespOk) {
		Server_Player *player = g->addPlayer(this, cmd->getSpectator());
		games.insert(cmd->getGameId(), QPair<Server_Game *, Server_Player *>(g, player));
		enqueueProtocolItem(new Event_GameJoined(cmd->getGameId(), g->getDescription(), player->getPlayerId(), cmd->getSpectator(), g->getSpectatorsCanTalk(), g->getSpectatorsSeeEverything(), false));
		enqueueProtocolItem(GameEventContainer::makeNew(new Event_GameStateChanged(g->getGameStarted(), g->getActivePlayer(), g->getActivePhase(), g->getGameState(player)), cmd->getGameId()));
	}
	return result;
}

ResponseCode Server_ProtocolHandler::cmdLeaveGame(Command_LeaveGame * /*cmd*/, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player)
{
	game->removePlayer(player);
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdKickFromGame(Command_KickFromGame *cmd, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player)
{
	if (game->getCreatorInfo()->getName() != player->getUserInfo()->getName())
		return RespFunctionNotAllowed;
	
	if (!game->kickPlayer(cmd->getPlayerId()))
		return RespNameNotFound;
	
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdDeckSelect(Command_DeckSelect *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	DeckList *deck;
	if (cmd->getDeckId() == -1) {
		if (!cmd->getDeck())
			return RespInvalidData;
		deck = new DeckList(cmd->getDeck());
	} else {
		try {
			deck = getDeckFromDatabase(cmd->getDeckId());
		} catch(ResponseCode r) {
			return r;
		}
	}
	player->setDeck(deck, cmd->getDeckId());
	
	game->sendGameEvent(new Event_PlayerPropertiesChanged(player->getPlayerId(), player->getProperties()), new Context_DeckSelect(cmd->getDeckId()));

	cont->setResponse(new Response_DeckDownload(cont->getCmdId(), RespOk, new DeckList(deck)));
	return RespNothing;
}

ResponseCode Server_ProtocolHandler::cmdSetSideboardPlan(Command_SetSideboardPlan *cmd, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	if (player->getReadyStart())
		return RespContextError;
	
	DeckList *deck = player->getDeck();
	if (!deck)
		return RespContextError;
	
	deck->setCurrentSideboardPlan(cmd->getMoveList());
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdConcede(Command_Concede * /*cmd*/, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player)
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

ResponseCode Server_ProtocolHandler::cmdReadyStart(Command_ReadyStart *cmd, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!player->getDeck() || game->getGameStarted())
		return RespContextError;

	if (player->getReadyStart() == cmd->getReady())
		return RespContextError;
	
	player->setReadyStart(cmd->getReady());
	game->sendGameEvent(new Event_PlayerPropertiesChanged(player->getPlayerId(), player->getProperties()), new Context_ReadyStart);
	game->startGameIfReady();
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdSay(Command_Say *cmd, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator() && !game->getSpectatorsCanTalk())
		return RespFunctionNotAllowed;
	
	game->sendGameEvent(new Event_Say(player->getPlayerId(), cmd->getMessage()));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdShuffle(Command_Shuffle * /*cmd*/, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player)
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

ResponseCode Server_ProtocolHandler::cmdMulligan(Command_Mulligan * /*cmd*/, CommandContainer *cont, Server_Game *game, Server_Player *player)
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
		CardToMove *cardToMove = new CardToMove(hand->cards.first()->getId());
		player->moveCard(cont, hand, QList<CardToMove *>() << cardToMove, deck, 0, 0, false);
		delete cardToMove;
	}

	deck->shuffle();
	cont->enqueueGameEventPrivate(new Event_Shuffle(player->getPlayerId()), game->getGameId());
	cont->enqueueGameEventOmniscient(new Event_Shuffle(player->getPlayerId()), game->getGameId());
	cont->enqueueGameEventPublic(new Event_Shuffle(player->getPlayerId()), game->getGameId());

	player->drawCards(cont, number);
	
	if (number == player->getInitialCards())
		number = -1;
	cont->getGameEventQueuePrivate()->setContext(new Context_Mulligan(number));
	cont->getGameEventQueuePublic()->setContext(new Context_Mulligan(number));
	cont->getGameEventQueueOmniscient()->setContext(new Context_Mulligan(number));

	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdRollDie(Command_RollDie *cmd, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	if (player->getConceded())
		return RespContextError;
	
	game->sendGameEvent(new Event_RollDie(player->getPlayerId(), cmd->getSides(), rng->getNumber(1, cmd->getSides())));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdDrawCards(Command_DrawCards *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
		
	return player->drawCards(cont, cmd->getNumber());
}

ResponseCode Server_ProtocolHandler::cmdUndoDraw(Command_UndoDraw * /*cmd*/, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
		
	return player->undoDraw(cont);
}

ResponseCode Server_ProtocolHandler::cmdMoveCard(Command_MoveCard *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
	
	return player->moveCard(cont, cmd->getStartZone(), cmd->getCards(), cmd->getTargetPlayerId(), cmd->getTargetZone(), cmd->getX(), cmd->getY());
}

ResponseCode Server_ProtocolHandler::cmdFlipCard(Command_FlipCard *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
	
	Server_CardZone *zone = player->getZones().value(cmd->getZone());
	if (!zone)
		return RespNameNotFound;
	if (!zone->hasCoords())
		return RespContextError;
	
	Server_Card *card = zone->getCard(cmd->getCardId());
	if (!card)
		return RespNameNotFound;
	
	const bool faceDown = cmd->getFaceDown();
	if (faceDown == card->getFaceDown())
		return RespContextError;
	
	card->setFaceDown(faceDown);
	cont->enqueueGameEventPrivate(new Event_FlipCard(player->getPlayerId(), zone->getName(), card->getId(), card->getName(), faceDown), game->getGameId());
	cont->enqueueGameEventPublic(new Event_FlipCard(player->getPlayerId(), zone->getName(), card->getId(), card->getName(), faceDown), game->getGameId());
	
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdAttachCard(Command_AttachCard *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
		
	Server_CardZone *startzone = player->getZones().value(cmd->getStartZone());
	if (!startzone)
		return RespNameNotFound;
	
	Server_Card *card = startzone->getCard(cmd->getCardId());
	if (!card)
		return RespNameNotFound;

	int playerId = cmd->getTargetPlayerId();
	Server_Player *targetPlayer = 0;
	Server_CardZone *targetzone = 0;
	Server_Card *targetCard = 0;
	
	if (playerId != -1) {
		targetPlayer = game->getPlayer(cmd->getTargetPlayerId());
		if (!targetPlayer)
			return RespNameNotFound;
	} else if (!card->getParentCard())
		return RespContextError;
	if (targetPlayer)
		targetzone = targetPlayer->getZones().value(cmd->getTargetZone());
	if (targetzone) {
		// This is currently enough to make sure cards don't get attached to a card that is not on the table.
		// Possibly a flag will have to be introduced for this sometime.
		if (!targetzone->hasCoords())
			return RespContextError;
		targetCard = targetzone->getCard(cmd->getTargetCardId());
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
			cont->enqueueGameEventPrivate(new Event_DeleteArrow(p->getPlayerId(), toDelete[i]->getId()), game->getGameId());
			cont->enqueueGameEventPublic(new Event_DeleteArrow(p->getPlayerId(), toDelete[i]->getId()), game->getGameId());
			p->deleteArrow(toDelete[i]->getId());
		}
	}

	if (targetCard) {
		// Unattach all cards attached to the card being attached.
		// Make a copy of the list because its contents change during the loop otherwise.
		QList<Server_Card *> attachedList = card->getAttachedCards();
		for (int i = 0; i < attachedList.size(); ++i)
			attachedList[i]->getZone()->getPlayer()->unattachCard(cont, attachedList[i]);
		
		if (targetzone->isColumnStacked(targetCard->getX(), targetCard->getY())) {
			CardToMove *cardToMove = new CardToMove(targetCard->getId());
			targetPlayer->moveCard(cont, targetzone, QList<CardToMove *>() << cardToMove, targetzone, targetzone->getFreeGridColumn(-2, targetCard->getY(), targetCard->getName()), targetCard->getY(), targetCard->getFaceDown());
			delete cardToMove;
		}
		
		card->setParentCard(targetCard);
		card->setCoords(-1, card->getY());
		cont->enqueueGameEventPrivate(new Event_AttachCard(player->getPlayerId(), startzone->getName(), card->getId(), targetPlayer->getPlayerId(), targetzone->getName(), targetCard->getId()), game->getGameId());
		cont->enqueueGameEventPublic(new Event_AttachCard(player->getPlayerId(), startzone->getName(), card->getId(), targetPlayer->getPlayerId(), targetzone->getName(), targetCard->getId()), game->getGameId());
		startzone->fixFreeSpaces(cont);
	} else
		player->unattachCard(cont, card);
	
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdCreateToken(Command_CreateToken *cmd, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
		
	Server_CardZone *zone = player->getZones().value(cmd->getZone());
	if (!zone)
		return RespNameNotFound;

	int x = cmd->getX();
	int y = cmd->getY();
	if (zone->hasCoords())
		x = zone->getFreeGridColumn(x, y, cmd->getCardName());
	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;

	Server_Card *card = new Server_Card(cmd->getCardName(), player->newCardId(), x, y);
	card->moveToThread(player->thread());
	card->setPT(cmd->getPt());
	card->setColor(cmd->getColor());
	card->setAnnotation(cmd->getAnnotation());
	card->setDestroyOnZoneChange(cmd->getDestroy());
	
	zone->insertCard(card, x, y);
	game->sendGameEvent(new Event_CreateToken(player->getPlayerId(), zone->getName(), card->getId(), card->getName(), cmd->getColor(), cmd->getPt(), cmd->getAnnotation(), cmd->getDestroy(), x, y));
	
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdCreateArrow(Command_CreateArrow *cmd, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
	
	Server_Player *startPlayer = game->getPlayer(cmd->getStartPlayerId());
	Server_Player *targetPlayer = game->getPlayer(cmd->getTargetPlayerId());
	if (!startPlayer || !targetPlayer)
		return RespNameNotFound;
	Server_CardZone *startZone = startPlayer->getZones().value(cmd->getStartZone());
	bool playerTarget = cmd->getTargetZone().isEmpty();
	Server_CardZone *targetZone = 0;
	if (!playerTarget)
		targetZone = targetPlayer->getZones().value(cmd->getTargetZone());
	if (!startZone || (!targetZone && !playerTarget))
		return RespNameNotFound;
	if (startZone->getType() != PublicZone)
		return RespContextError;
	Server_Card *startCard = startZone->getCard(cmd->getStartCardId());
	if (!startCard)
		return RespNameNotFound;
	Server_Card *targetCard = 0;
	if (!playerTarget) {
		if (targetZone->getType() != PublicZone)
			return RespContextError;
		targetCard = targetZone->getCard(cmd->getTargetCardId());
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
	
	Server_Arrow *arrow = new Server_Arrow(player->newArrowId(), startCard, targetItem, cmd->getColor());
	player->addArrow(arrow);
	game->sendGameEvent(new Event_CreateArrows(player->getPlayerId(), QList<ServerInfo_Arrow *>() << new ServerInfo_Arrow(
		arrow->getId(),
		startPlayer->getPlayerId(),
		startZone->getName(),
		startCard->getId(),
		targetPlayer->getPlayerId(),
		cmd->getTargetZone(),
		cmd->getTargetCardId(),
		cmd->getColor()
	)));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdDeleteArrow(Command_DeleteArrow *cmd, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
	
	if (!player->deleteArrow(cmd->getArrowId()))
		return RespNameNotFound;
	
	game->sendGameEvent(new Event_DeleteArrow(player->getPlayerId(), cmd->getArrowId()));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdSetCardAttr(Command_SetCardAttr *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
	
	return player->setCardAttrHelper(cont, cmd->getZone(), cmd->getCardId(), cmd->getAttrName(), cmd->getAttrValue());
}

ResponseCode Server_ProtocolHandler::cmdSetCardCounter(Command_SetCardCounter *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
	
	Server_CardZone *zone = player->getZones().value(cmd->getZone());
	if (!zone)
		return RespNameNotFound;
	if (!zone->hasCoords())
		return RespContextError;

	Server_Card *card = zone->getCard(cmd->getCardId());
	if (!card)
		return RespNameNotFound;
	
	card->setCounter(cmd->getCounterId(), cmd->getCounterValue());
	
	cont->enqueueGameEventPrivate(new Event_SetCardCounter(player->getPlayerId(), zone->getName(), card->getId(), cmd->getCounterId(), cmd->getCounterValue()), game->getGameId());
	cont->enqueueGameEventPublic(new Event_SetCardCounter(player->getPlayerId(), zone->getName(), card->getId(), cmd->getCounterId(), cmd->getCounterValue()), game->getGameId());
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdIncCardCounter(Command_IncCardCounter *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
	
	Server_CardZone *zone = player->getZones().value(cmd->getZone());
	if (!zone)
		return RespNameNotFound;
	if (!zone->hasCoords())
		return RespContextError;

	Server_Card *card = zone->getCard(cmd->getCardId());
	if (!card)
		return RespNameNotFound;
	
	int newValue = card->getCounter(cmd->getCounterId()) + cmd->getCounterDelta();
	card->setCounter(cmd->getCounterId(), newValue);
	
	cont->enqueueGameEventPrivate(new Event_SetCardCounter(player->getPlayerId(), zone->getName(), card->getId(), cmd->getCounterId(), newValue), game->getGameId());
	cont->enqueueGameEventPublic(new Event_SetCardCounter(player->getPlayerId(), zone->getName(), card->getId(), cmd->getCounterId(), newValue), game->getGameId());
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdIncCounter(Command_IncCounter *cmd, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
	
	const QMap<int, Server_Counter *> counters = player->getCounters();
	Server_Counter *c = counters.value(cmd->getCounterId(), 0);
	if (!c)
		return RespNameNotFound;
	
	c->setCount(c->getCount() + cmd->getDelta());
	game->sendGameEvent(new Event_SetCounter(player->getPlayerId(), c->getId(), c->getCount()));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdCreateCounter(Command_CreateCounter *cmd, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
	
	Server_Counter *c = new Server_Counter(player->newCounterId(), cmd->getCounterName(), cmd->getColor(), cmd->getRadius(), cmd->getValue());
	player->addCounter(c);
	game->sendGameEvent(new Event_CreateCounters(player->getPlayerId(), QList<ServerInfo_Counter *>() << new ServerInfo_Counter(c->getId(), c->getName(), c->getColor(), c->getRadius(), c->getCount())));
	
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdSetCounter(Command_SetCounter *cmd, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
	
	Server_Counter *c = player->getCounters().value(cmd->getCounterId(), 0);;
	if (!c)
		return RespNameNotFound;
	
	c->setCount(cmd->getValue());
	game->sendGameEvent(new Event_SetCounter(player->getPlayerId(), c->getId(), c->getCount()));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdDelCounter(Command_DelCounter *cmd, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
	
	if (!player->deleteCounter(cmd->getCounterId()))
		return RespNameNotFound;
	game->sendGameEvent(new Event_DelCounter(player->getPlayerId(), cmd->getCounterId()));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdNextTurn(Command_NextTurn * /*cmd*/, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player)
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

ResponseCode Server_ProtocolHandler::cmdSetActivePhase(Command_SetActivePhase *cmd, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
	
	if (game->getActivePlayer() != player->getPlayerId())
		return RespContextError;
	game->setActivePhase(cmd->getPhase());
	
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdDumpZone(Command_DumpZone *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (!game->getGameStarted())
		return RespGameNotStarted;
	
	Server_Player *otherPlayer = game->getPlayer(cmd->getPlayerId());
	if (!otherPlayer)
		return RespNameNotFound;
	Server_CardZone *zone = otherPlayer->getZones().value(cmd->getZoneName());
	if (!zone)
		return RespNameNotFound;
	if (!((zone->getType() == PublicZone) || (player == otherPlayer)))
		return RespContextError;
	
	int numberCards = cmd->getNumberCards();
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
	cont->setResponse(new Response_DumpZone(cont->getCmdId(), RespOk, new ServerInfo_Zone(zone->getName(), zone->getType(), zone->hasCoords(), numberCards < zone->cards.size() ? zone->cards.size() : numberCards, respCardList)));
	return RespNothing;
}

ResponseCode Server_ProtocolHandler::cmdStopDumpZone(Command_StopDumpZone *cmd, CommandContainer * /*cont*/, Server_Game *game, Server_Player *player)
{
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
	
	Server_Player *otherPlayer = game->getPlayer(cmd->getPlayerId());
	if (!otherPlayer)
		return RespNameNotFound;
	Server_CardZone *zone = otherPlayer->getZones().value(cmd->getZoneName());
	if (!zone)
		return RespNameNotFound;
	
	if (zone->getType() == HiddenZone) {
		zone->setCardsBeingLookedAt(0);
		game->sendGameEvent(new Event_StopDumpZone(player->getPlayerId(), cmd->getPlayerId(), zone->getName()));
	}
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdRevealCards(Command_RevealCards *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	if (player->getConceded())
		return RespContextError;
	
	Server_Player *otherPlayer = 0;
	if (cmd->getPlayerId() != -1) {
		otherPlayer = game->getPlayer(cmd->getPlayerId());
		if (!otherPlayer)
			return RespNameNotFound;
	}
	Server_CardZone *zone = player->getZones().value(cmd->getZoneName());
	if (!zone)
		return RespNameNotFound;
	
	QList<Server_Card *> cardsToReveal;
	if (cmd->getCardId() == -1)
		cardsToReveal = zone->cards;
	else if (cmd->getCardId() == -2) {
		if (zone->cards.isEmpty())
			return RespContextError;
		cardsToReveal.append(zone->cards.at(rng->getNumber(0, zone->cards.size() - 1)));
	} else {
		Server_Card *card = zone->getCard(cmd->getCardId());
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
		
		if (cmd->getPlayerId() != -1)
			respCardListPrivate.append(new ServerInfo_Card(card->getId(), card->getName(), card->getX(), card->getY(), card->getFaceDown(), card->getTapped(), card->getAttacking(), card->getColor(), card->getPT(), card->getAnnotation(), card->getDestroyOnZoneChange(), card->getDoesntUntap(), cardCounterListPrivate, attachPlayerId, attachZone, attachCardId));
		respCardListOmniscient.append(new ServerInfo_Card(card->getId(), card->getName(), card->getX(), card->getY(), card->getFaceDown(), card->getTapped(), card->getAttacking(), card->getColor(), card->getPT(), card->getAnnotation(), card->getDestroyOnZoneChange(), card->getDoesntUntap(), cardCounterListOmniscient, attachPlayerId, attachZone, attachCardId));
	}
	
	if (cmd->getPlayerId() == -1)
		cont->enqueueGameEventPublic(new Event_RevealCards(player->getPlayerId(), zone->getName(), cmd->getCardId(), -1, respCardListOmniscient), game->getGameId());
	else {
		cont->enqueueGameEventPublic(new Event_RevealCards(player->getPlayerId(), zone->getName(), cmd->getCardId(), otherPlayer->getPlayerId()), game->getGameId());
		cont->enqueueGameEventPrivate(new Event_RevealCards(player->getPlayerId(), zone->getName(), cmd->getCardId(), otherPlayer->getPlayerId(), respCardListPrivate), game->getGameId(), otherPlayer->getPlayerId());
		cont->enqueueGameEventOmniscient(new Event_RevealCards(player->getPlayerId(), zone->getName(), cmd->getCardId(), otherPlayer->getPlayerId(), respCardListOmniscient), game->getGameId());
	}
	
	return RespOk;
}
