import type {
  Response_GetGamesOfUser,
  Response_DeckList,
  Response_DeckDownload,
  Response_ReplayDownload,
  Response_WarnList,
  Event_RoomSay,
  Event_GameJoined,
  Event_GameStateChanged,
  Event_MoveCard,
  Event_FlipCard,
  Event_DestroyCard,
  Event_AttachCard,
  Event_CreateToken,
  Event_SetCardAttr,
  Event_SetCardCounter,
  Event_CreateArrow,
  Event_DeleteArrow,
  Event_CreateCounter,
  Event_SetCounter,
  Event_DelCounter,
  Event_DrawCards,
  Event_RevealCards,
  Event_Shuffle,
  Event_RollDie,
  Event_DumpZone,
  Event_ChangeZoneProperties,
  Event_NotifyUser,
  Event_PlayerPropertiesChanged,
  Event_ServerShutdown,
  Event_UserMessage,
  RoomEventMap,
  ServerInfo_User,
  ServerInfo_Room,
  ServerInfo_Game,
  ServerInfo_PlayerProperties,
  ServerInfo_Ban,
  ServerInfo_ChatMessage,
  ServerInfo_Warning,
  ServerInfo_DeckStorage_TreeItem,
  ServerInfo_ReplayMatch,
} from '@app/generated';

import type { StatusEnum } from './StatusEnum';
import type { LoginSuccessContext, PendingActivationContext } from './SignalContexts';
import type {
  KeyOf,
  WebSocketRoomResponseOverrides,
} from './WebSocketConfig';

export interface ISessionResponse {
  initialized(): void;
  connectionAttempted(): void;
  clearStore(): void;
  loginSuccessful(options: LoginSuccessContext): void;
  loginFailed(): void;
  connectionFailed(): void;
  testConnectionSuccessful(): void;
  testConnectionFailed(): void;
  updateBuddyList(buddyList: ServerInfo_User[]): void;
  addToBuddyList(user: ServerInfo_User): void;
  removeFromBuddyList(userName: string): void;
  updateIgnoreList(ignoreList: ServerInfo_User[]): void;
  addToIgnoreList(user: ServerInfo_User): void;
  removeFromIgnoreList(userName: string): void;
  updateInfo(name: string, version: string): void;
  updateStatus(state: StatusEnum, description: string): void;
  updateUser(user: ServerInfo_User): void;
  updateUsers(users: ServerInfo_User[]): void;
  userJoined(user: ServerInfo_User): void;
  userLeft(userName: string): void;
  serverMessage(message: string): void;
  accountAwaitingActivation(options: PendingActivationContext): void;
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
  getUserInfo(userInfo: ServerInfo_User): void;
  getGamesOfUser(userName: string, response: Response_GetGamesOfUser): void;
  gameJoined(gameJoinedData: Event_GameJoined): void;
  notifyUser(notification: Event_NotifyUser): void;
  playerPropertiesChanged(gameId: number, playerId: number, payload: Event_PlayerPropertiesChanged): void;
  serverShutdown(data: Event_ServerShutdown): void;
  userMessage(messageData: Event_UserMessage): void;
  addToList(list: string, userName: string): void;
  removeFromList(list: string, userName: string): void;
  deleteServerDeck(deckId: number): void;
  updateServerDecks(deckList: Response_DeckList): void;
  uploadServerDeck(path: string, treeItem: ServerInfo_DeckStorage_TreeItem): void;
  downloadServerDeck(deckId: number, response: Response_DeckDownload): void;
  createServerDeckDir(path: string, dirName: string): void;
  deleteServerDeckDir(path: string): void;
  replayList(matchList: ServerInfo_ReplayMatch[]): void;
  replayAdded(matchInfo: ServerInfo_ReplayMatch): void;
  replayModifyMatch(gameId: number, doNotHide: boolean): void;
  replayDeleteMatch(gameId: number): void;
  replayDownloaded(replayId: number, response: Response_ReplayDownload): void;
}

export interface IRoomResponse<T extends RoomEventMap = WebSocketRoomResponseOverrides> {
  clearStore(): void;
  joinRoom(roomInfo: ServerInfo_Room): void;
  leaveRoom(roomId: number): void;
  updateRooms(rooms: ServerInfo_Room[]): void;
  updateGames(roomId: number, gameList: ServerInfo_Game[]): void;
  addMessage(roomId: number, message: T[KeyOf<RoomEventMap, Event_RoomSay>]): void;
  userJoined(roomId: number, user: ServerInfo_User): void;
  userLeft(roomId: number, name: string): void;
  removeMessages(roomId: number, name: string, amount: number): void;
  gameCreated(roomId: number): void;
  joinedGame(roomId: number, gameId: number): void;
  setJoinGamePending(pending: boolean): void;
  setJoinGameError(code: number, message: string): void;
}

export interface IGameResponse {
  clearStore(): void;
  gameStateChanged(gameId: number, data: Event_GameStateChanged): void;
  playerJoined(gameId: number, playerProperties: ServerInfo_PlayerProperties): void;
  playerLeft(gameId: number, playerId: number, reason: number): void;
  playerPropertiesChanged(gameId: number, playerId: number, properties: ServerInfo_PlayerProperties): void;
  gameClosed(gameId: number): void;
  gameHostChanged(gameId: number, hostId: number): void;
  kicked(gameId: number): void;
  gameSay(gameId: number, playerId: number, message: string, timeReceived: number): void;
  cardMoved(gameId: number, playerId: number, data: Event_MoveCard): void;
  cardFlipped(gameId: number, playerId: number, data: Event_FlipCard): void;
  cardDestroyed(gameId: number, playerId: number, data: Event_DestroyCard): void;
  cardAttached(gameId: number, playerId: number, data: Event_AttachCard): void;
  tokenCreated(gameId: number, playerId: number, data: Event_CreateToken): void;
  cardAttrChanged(gameId: number, playerId: number, data: Event_SetCardAttr): void;
  cardCounterChanged(gameId: number, playerId: number, data: Event_SetCardCounter): void;
  arrowCreated(gameId: number, playerId: number, data: Event_CreateArrow): void;
  arrowDeleted(gameId: number, playerId: number, data: Event_DeleteArrow): void;
  counterCreated(gameId: number, playerId: number, data: Event_CreateCounter): void;
  counterSet(gameId: number, playerId: number, data: Event_SetCounter): void;
  counterDeleted(gameId: number, playerId: number, data: Event_DelCounter): void;
  cardsDrawn(gameId: number, playerId: number, data: Event_DrawCards): void;
  cardsRevealed(gameId: number, playerId: number, data: Event_RevealCards): void;
  zoneShuffled(gameId: number, playerId: number, data: Event_Shuffle): void;
  dieRolled(gameId: number, playerId: number, data: Event_RollDie): void;
  activePlayerSet(gameId: number, activePlayerId: number): void;
  activePhaseSet(gameId: number, phase: number): void;
  turnReversed(gameId: number, reversed: boolean): void;
  zoneDumped(gameId: number, playerId: number, data: Event_DumpZone): void;
  zonePropertiesChanged(gameId: number, playerId: number, data: Event_ChangeZoneProperties): void;
}

export interface IAdminResponse {
  adjustMod(userName: string, shouldBeMod: boolean, shouldBeJudge: boolean): void;
  reloadConfig(): void;
  shutdownServer(): void;
  updateServerMessage(): void;
}

export interface IModeratorResponse {
  banFromServer(userName: string): void;
  banHistory(userName: string, banHistory: ServerInfo_Ban[]): void;
  viewLogs(logs: ServerInfo_ChatMessage[]): void;
  warnHistory(userName: string, warnHistory: ServerInfo_Warning[]): void;
  warnListOptions(warnList: Response_WarnList[]): void;
  warnUser(userName: string): void;
  grantReplayAccess(replayId: number, moderatorName: string): void;
  forceActivateUser(usernameToActivate: string, moderatorName: string): void;
  getAdminNotes(userName: string, notes: string): void;
  updateAdminNotes(userName: string, notes: string): void;
}

export interface IWebClientResponse<
  R extends RoomEventMap = WebSocketRoomResponseOverrides,
> {
  session: ISessionResponse;
  room: IRoomResponse<R>;
  game: IGameResponse;
  admin: IAdminResponse;
  moderator: IModeratorResponse;
}
