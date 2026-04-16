import { App, Data, Enriched } from '@app/types';

export interface ISessionResponse {
  initialized(): void;
  connectionAttempted(): void;
  clearStore(): void;
  loginSuccessful(options: Enriched.LoginSuccessContext): void;
  loginFailed(): void;
  connectionFailed(): void;
  testConnectionSuccessful(): void;
  testConnectionFailed(): void;
  updateBuddyList(buddyList: Data.ServerInfo_User[]): void;
  addToBuddyList(user: Data.ServerInfo_User): void;
  removeFromBuddyList(userName: string): void;
  updateIgnoreList(ignoreList: Data.ServerInfo_User[]): void;
  addToIgnoreList(user: Data.ServerInfo_User): void;
  removeFromIgnoreList(userName: string): void;
  updateInfo(name: string, version: string): void;
  updateStatus(state: App.StatusEnum, description: string): void;
  updateUser(user: Data.ServerInfo_User): void;
  updateUsers(users: Data.ServerInfo_User[]): void;
  userJoined(user: Data.ServerInfo_User): void;
  userLeft(userName: string): void;
  serverMessage(message: string): void;
  accountAwaitingActivation(options: Enriched.PendingActivationContext): void;
  accountActivationSuccess(): void;
  accountActivationFailed(): void;
  registrationRequiresEmail(): void;
  registrationSuccess(): void;
  registrationFailed(reason: string, endTime?: number): void;
  registrationEmailError(error: string): void;
  registrationPasswordError(error: string): void;
  registrationUserNameError(error: string): void;
  resetPasswordChallenge(): void;
  resetPassword(): void;
  resetPasswordSuccess(): void;
  resetPasswordFailed(): void;
  accountPasswordChange(): void;
  accountEditChanged(realName?: string, email?: string, country?: string): void;
  accountImageChanged(avatarBmp: Uint8Array): void;
  getUserInfo(userInfo: Data.ServerInfo_User): void;
  getGamesOfUser(userName: string, response: Data.Response_GetGamesOfUser): void;
  gameJoined(gameJoinedData: Data.Event_GameJoined): void;
  notifyUser(notification: Data.Event_NotifyUser): void;
  playerPropertiesChanged(gameId: number, playerId: number, payload: Data.Event_PlayerPropertiesChanged): void;
  serverShutdown(data: Data.Event_ServerShutdown): void;
  userMessage(messageData: Data.Event_UserMessage): void;
  addToList(list: string, userName: string): void;
  removeFromList(list: string, userName: string): void;
  deleteServerDeck(deckId: number): void;
  updateServerDecks(deckList: Data.Response_DeckList): void;
  uploadServerDeck(path: string, treeItem: Data.ServerInfo_DeckStorage_TreeItem): void;
  createServerDeckDir(path: string, dirName: string): void;
  deleteServerDeckDir(path: string): void;
  replayList(matchList: Data.ServerInfo_ReplayMatch[]): void;
  replayAdded(matchInfo: Data.ServerInfo_ReplayMatch): void;
  replayModifyMatch(gameId: number, doNotHide: boolean): void;
  replayDeleteMatch(gameId: number): void;
}

export interface IRoomResponse {
  clearStore(): void;
  joinRoom(roomInfo: Data.ServerInfo_Room): void;
  leaveRoom(roomId: number): void;
  updateRooms(rooms: Data.ServerInfo_Room[]): void;
  updateGames(roomId: number, gameList: Data.ServerInfo_Game[]): void;
  addMessage(roomId: number, message: Enriched.Message): void;
  userJoined(roomId: number, user: Data.ServerInfo_User): void;
  userLeft(roomId: number, name: string): void;
  removeMessages(roomId: number, name: string, amount: number): void;
  gameCreated(roomId: number): void;
  joinedGame(roomId: number, gameId: number): void;
}

export interface IGameResponse {
  clearStore(): void;
  gameStateChanged(gameId: number, data: Data.Event_GameStateChanged): void;
  playerJoined(gameId: number, playerProperties: Data.ServerInfo_PlayerProperties): void;
  playerLeft(gameId: number, playerId: number, reason: number): void;
  playerPropertiesChanged(gameId: number, playerId: number, properties: Data.ServerInfo_PlayerProperties): void;
  gameClosed(gameId: number): void;
  gameHostChanged(gameId: number, hostId: number): void;
  kicked(gameId: number): void;
  gameSay(gameId: number, playerId: number, message: string): void;
  cardMoved(gameId: number, playerId: number, data: Data.Event_MoveCard): void;
  cardFlipped(gameId: number, playerId: number, data: Data.Event_FlipCard): void;
  cardDestroyed(gameId: number, playerId: number, data: Data.Event_DestroyCard): void;
  cardAttached(gameId: number, playerId: number, data: Data.Event_AttachCard): void;
  tokenCreated(gameId: number, playerId: number, data: Data.Event_CreateToken): void;
  cardAttrChanged(gameId: number, playerId: number, data: Data.Event_SetCardAttr): void;
  cardCounterChanged(gameId: number, playerId: number, data: Data.Event_SetCardCounter): void;
  arrowCreated(gameId: number, playerId: number, data: Data.Event_CreateArrow): void;
  arrowDeleted(gameId: number, playerId: number, data: Data.Event_DeleteArrow): void;
  counterCreated(gameId: number, playerId: number, data: Data.Event_CreateCounter): void;
  counterSet(gameId: number, playerId: number, data: Data.Event_SetCounter): void;
  counterDeleted(gameId: number, playerId: number, data: Data.Event_DelCounter): void;
  cardsDrawn(gameId: number, playerId: number, data: Data.Event_DrawCards): void;
  cardsRevealed(gameId: number, playerId: number, data: Data.Event_RevealCards): void;
  zoneShuffled(gameId: number, playerId: number, data: Data.Event_Shuffle): void;
  dieRolled(gameId: number, playerId: number, data: Data.Event_RollDie): void;
  activePlayerSet(gameId: number, activePlayerId: number): void;
  activePhaseSet(gameId: number, phase: number): void;
  turnReversed(gameId: number, reversed: boolean): void;
  zoneDumped(gameId: number, playerId: number, data: Data.Event_DumpZone): void;
  zonePropertiesChanged(gameId: number, playerId: number, data: Data.Event_ChangeZoneProperties): void;
}

export interface IAdminResponse {
  adjustMod(userName: string, shouldBeMod: boolean, shouldBeJudge: boolean): void;
  reloadConfig(): void;
  shutdownServer(): void;
  updateServerMessage(): void;
}

export interface IModeratorResponse {
  banFromServer(userName: string): void;
  banHistory(userName: string, banHistory: Data.ServerInfo_Ban[]): void;
  viewLogs(logs: Data.ServerInfo_ChatMessage[]): void;
  warnHistory(userName: string, warnHistory: Data.ServerInfo_Warning[]): void;
  warnListOptions(warnList: Data.Response_WarnList[]): void;
  warnUser(userName: string): void;
  grantReplayAccess(replayId: number, moderatorName: string): void;
  forceActivateUser(usernameToActivate: string, moderatorName: string): void;
  getAdminNotes(userName: string, notes: string): void;
  updateAdminNotes(userName: string, notes: string): void;
}

export interface IWebClientResponse {
  session: ISessionResponse;
  room: IRoomResponse;
  game: IGameResponse;
  admin: IAdminResponse;
  moderator: IModeratorResponse;
}
