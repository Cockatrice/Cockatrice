import type {
  LoginParams,
  RegisterParams,
  ActivateParams,
  ForgotPasswordRequestParams,
  ForgotPasswordChallengeParams,
  ForgotPasswordResetParams,
  ViewLogHistoryParams,
  KickFromGameParams,
  GameSayParams,
  ReadyStartParams,
  SetActivePhaseParams,
  MoveCardParams,
  FlipCardParams,
  AttachCardParams,
  CreateTokenParams,
  SetCardAttrParams,
  SetCardCounterParams,
  IncCardCounterParams,
  DrawCardsParams,
  CreateArrowParams,
  DeleteArrowParams,
  CreateCounterParams,
  SetCounterParams,
  IncCounterParams,
  DelCounterParams,
  ShuffleParams,
  DumpZoneParams,
  RevealCardsParams,
  ChangeZonePropertiesParams,
  DeckSelectParams,
  SetSideboardPlanParams,
  SetSideboardLockParams,
  MulliganParams,
  RollDieParams,
  GameCommand,
} from '@app/generated';

import type { ConnectTarget } from '../WebClientConfig';
import type { KeyOf } from '../types';

// ── Auth request type map ────────────────────────────────────────────────────
// Keys = generated *Params type names composed with ConnectTarget.
// @app/api overrides these with Enriched connect option types.

export interface AuthRequestMap {
  LoginParams: ConnectTarget & LoginParams;
  ConnectTarget: ConnectTarget;
  RegisterParams: ConnectTarget & RegisterParams;
  ActivateParams: ConnectTarget & ActivateParams;
  ForgotPasswordRequestParams: ConnectTarget & ForgotPasswordRequestParams;
  ForgotPasswordChallengeParams: ConnectTarget & ForgotPasswordChallengeParams;
  ForgotPasswordResetParams: ConnectTarget & ForgotPasswordResetParams;
}

type AK<V> = KeyOf<AuthRequestMap, V>;

export interface IAuthenticationRequest<T extends AuthRequestMap = AuthRequestMap> {
  login(options: T[AK<ConnectTarget & LoginParams>]): void;
  testConnection(options: T[AK<ConnectTarget>]): void;
  register(options: T[AK<ConnectTarget & RegisterParams>]): void;
  activateAccount(options: T[AK<ConnectTarget & ActivateParams>]): void;
  resetPasswordRequest(options: T[AK<ConnectTarget & ForgotPasswordRequestParams>]): void;
  resetPasswordChallenge(options: T[AK<ConnectTarget & ForgotPasswordChallengeParams>]): void;
  resetPassword(options: T[AK<ConnectTarget & ForgotPasswordResetParams>]): void;
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
  viewLogHistory(filters: ViewLogHistoryParams): void;
  warnUser(userName: string, reason: string, clientid?: string, removeMessages?: number): void;
}

export interface IGameRequest {
  leaveGame(gameId: number): void;
  kickFromGame(gameId: number, params: KickFromGameParams): void;
  gameSay(gameId: number, params: GameSayParams): void;
  readyStart(gameId: number, params: ReadyStartParams): void;
  concede(gameId: number): void;
  unconcede(gameId: number): void;
  judge(gameId: number, targetId: number, innerGameCommand: GameCommand): void;
  nextTurn(gameId: number): void;
  setActivePhase(gameId: number, params: SetActivePhaseParams): void;
  reverseTurn(gameId: number): void;
  moveCard(gameId: number, params: MoveCardParams): void;
  flipCard(gameId: number, params: FlipCardParams): void;
  attachCard(gameId: number, params: AttachCardParams): void;
  createToken(gameId: number, params: CreateTokenParams): void;
  setCardAttr(gameId: number, params: SetCardAttrParams): void;
  setCardCounter(gameId: number, params: SetCardCounterParams): void;
  incCardCounter(gameId: number, params: IncCardCounterParams): void;
  drawCards(gameId: number, params: DrawCardsParams): void;
  undoDraw(gameId: number): void;
  createArrow(gameId: number, params: CreateArrowParams): void;
  deleteArrow(gameId: number, params: DeleteArrowParams): void;
  createCounter(gameId: number, params: CreateCounterParams): void;
  setCounter(gameId: number, params: SetCounterParams): void;
  incCounter(gameId: number, params: IncCounterParams): void;
  delCounter(gameId: number, params: DelCounterParams): void;
  shuffle(gameId: number, params: ShuffleParams): void;
  dumpZone(gameId: number, params: DumpZoneParams): void;
  revealCards(gameId: number, params: RevealCardsParams): void;
  changeZoneProperties(gameId: number, params: ChangeZonePropertiesParams): void;
  deckSelect(gameId: number, params: DeckSelectParams): void;
  setSideboardPlan(gameId: number, params: SetSideboardPlanParams): void;
  setSideboardLock(gameId: number, params: SetSideboardLockParams): void;
  mulligan(gameId: number, params: MulliganParams): void;
  rollDie(gameId: number, params: RollDieParams): void;
}

export interface IWebClientRequest<
  A extends AuthRequestMap = AuthRequestMap,
> {
  authentication: IAuthenticationRequest<A>;
  session: ISessionRequest;
  rooms: IRoomsRequest;
  game: IGameRequest;
  admin: IAdminRequest;
  moderator: IModeratorRequest;
}
