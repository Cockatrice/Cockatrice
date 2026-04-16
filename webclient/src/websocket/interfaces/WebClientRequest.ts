import { Data, Enriched } from '@app/types';

export interface IAuthenticationRequest {
  login(options: Omit<Enriched.LoginConnectOptions, 'reason'>): void;
  testConnection(options: Omit<Enriched.TestConnectionOptions, 'reason'>): void;
  register(options: Omit<Enriched.RegisterConnectOptions, 'reason'>): void;
  activateAccount(options: Omit<Enriched.ActivateConnectOptions, 'reason'>): void;
  resetPasswordRequest(options: Omit<Enriched.PasswordResetRequestConnectOptions, 'reason'>): void;
  resetPasswordChallenge(options: Omit<Enriched.PasswordResetChallengeConnectOptions, 'reason'>): void;
  resetPassword(options: Omit<Enriched.PasswordResetConnectOptions, 'reason'>): void;
  disconnect(): void;
}

export interface ISessionRequest {
  addToBuddyList(userName: string): void;
  removeFromBuddyList(userName: string): void;
  addToIgnoreList(userName: string): void;
  removeFromIgnoreList(userName: string): void;
  changeAccountPassword(oldPassword: string, newPassword: string, hashedNewPassword?: string): void;
  changeAccountDetails(passwordCheck: string, realName?: string, email?: string, country?: string): void;
  changeAccountImage(image: Uint8Array): void;
  sendDirectMessage(userName: string, message: string): void;
  getUserInfo(userName: string): void;
  getUserGames(userName: string): void;
  deckDownload(deckId: number): void;
  replayDownload(replayId: number): void;
}

export interface IRoomsRequest {
  joinRoom(roomId: number): void;
  leaveRoom(roomId: number): void;
  roomSay(roomId: number, message: string): void;
}

export interface IAdminRequest {
  adjustMod(userName: string, shouldBeMod?: boolean, shouldBeJudge?: boolean): void;
  reloadConfig(): void;
  shutdownServer(reason: string, minutes: number): void;
  updateServerMessage(): void;
}

export interface IModeratorRequest {
  banFromServer(
    minutes: number,
    userName?: string,
    address?: string,
    reason?: string,
    visibleReason?: string,
    clientid?: string,
    removeMessages?: number
  ): void;
  getBanHistory(userName: string): void;
  getWarnHistory(userName: string): void;
  getWarnList(modName: string, userName: string, userClientid: string): void;
  viewLogHistory(filters: Data.ViewLogHistoryParams): void;
  warnUser(userName: string, reason: string, clientid?: string, removeMessages?: number): void;
}

export interface IGameRequest {
  leaveGame(gameId: number): void;
  kickFromGame(gameId: number, params: Data.KickFromGameParams): void;
  gameSay(gameId: number, params: Data.GameSayParams): void;
  readyStart(gameId: number, params: Data.ReadyStartParams): void;
  concede(gameId: number): void;
  unconcede(gameId: number): void;
  judge(gameId: number, targetId: number, innerGameCommand: Data.GameCommand): void;
  nextTurn(gameId: number): void;
  setActivePhase(gameId: number, params: Data.SetActivePhaseParams): void;
  reverseTurn(gameId: number): void;
  moveCard(gameId: number, params: Data.MoveCardParams): void;
  flipCard(gameId: number, params: Data.FlipCardParams): void;
  attachCard(gameId: number, params: Data.AttachCardParams): void;
  createToken(gameId: number, params: Data.CreateTokenParams): void;
  setCardAttr(gameId: number, params: Data.SetCardAttrParams): void;
  setCardCounter(gameId: number, params: Data.SetCardCounterParams): void;
  incCardCounter(gameId: number, params: Data.IncCardCounterParams): void;
  drawCards(gameId: number, params: Data.DrawCardsParams): void;
  undoDraw(gameId: number): void;
  createArrow(gameId: number, params: Data.CreateArrowParams): void;
  deleteArrow(gameId: number, params: Data.DeleteArrowParams): void;
  createCounter(gameId: number, params: Data.CreateCounterParams): void;
  setCounter(gameId: number, params: Data.SetCounterParams): void;
  incCounter(gameId: number, params: Data.IncCounterParams): void;
  delCounter(gameId: number, params: Data.DelCounterParams): void;
  shuffle(gameId: number, params: Data.ShuffleParams): void;
  dumpZone(gameId: number, params: Data.DumpZoneParams): void;
  revealCards(gameId: number, params: Data.RevealCardsParams): void;
  changeZoneProperties(gameId: number, params: Data.ChangeZonePropertiesParams): void;
  deckSelect(gameId: number, params: Data.DeckSelectParams): void;
  setSideboardPlan(gameId: number, params: Data.SetSideboardPlanParams): void;
  setSideboardLock(gameId: number, params: Data.SetSideboardLockParams): void;
  mulligan(gameId: number, params: Data.MulliganParams): void;
  rollDie(gameId: number, params: Data.RollDieParams): void;
}

export interface IWebClientRequest {
  authentication: IAuthenticationRequest;
  session: ISessionRequest;
  rooms: IRoomsRequest;
  game: IGameRequest;
  admin: IAdminRequest;
  moderator: IModeratorRequest;
}
