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

export interface IWebClientRequest {
  authentication: IAuthenticationRequest;
  session: ISessionRequest;
  rooms: IRoomsRequest;
  admin: IAdminRequest;
  moderator: IModeratorRequest;
}
