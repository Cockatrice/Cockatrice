import { Data } from '@app/types';
import type { ISessionResponse, WebSocketSessionResponseOverrides } from '@app/websocket';
import { StatusEnum } from '@app/websocket';
import { GameDispatch, RoomsDispatch, ServerDispatch } from '@app/store';

type LoginSuccess = WebSocketSessionResponseOverrides['Response_Login'];
type PendingActivation = WebSocketSessionResponseOverrides['Response'];

export class SessionResponseImpl implements ISessionResponse<WebSocketSessionResponseOverrides> {
  initialized(): void {
    ServerDispatch.initialized();
  }

  connectionAttempted(): void {
    ServerDispatch.connectionAttempted();
  }

  clearStore(): void {
    ServerDispatch.clearStore();
  }

  loginSuccessful(options: LoginSuccess): void {
    ServerDispatch.loginSuccessful(options);
  }

  loginFailed(): void {
    ServerDispatch.loginFailed();
  }

  connectionFailed(): void {
    ServerDispatch.connectionFailed();
  }

  testConnectionSuccessful(): void {
    ServerDispatch.testConnectionSuccessful();
  }

  testConnectionFailed(): void {
    ServerDispatch.testConnectionFailed();
  }

  updateBuddyList(buddyList: Data.ServerInfo_User[]): void {
    ServerDispatch.updateBuddyList(buddyList);
  }

  addToBuddyList(user: Data.ServerInfo_User): void {
    ServerDispatch.addToBuddyList(user);
  }

  removeFromBuddyList(userName: string): void {
    ServerDispatch.removeFromBuddyList(userName);
  }

  updateIgnoreList(ignoreList: Data.ServerInfo_User[]): void {
    ServerDispatch.updateIgnoreList(ignoreList);
  }

  addToIgnoreList(user: Data.ServerInfo_User): void {
    ServerDispatch.addToIgnoreList(user);
  }

  removeFromIgnoreList(userName: string): void {
    ServerDispatch.removeFromIgnoreList(userName);
  }

  updateInfo(name: string, version: string): void {
    ServerDispatch.updateInfo(name, version);
  }

  updateStatus(state: StatusEnum, description: string): void {
    if (state === StatusEnum.DISCONNECTED) {
      GameDispatch.clearStore();
      RoomsDispatch.clearStore();
      ServerDispatch.clearStore();
    }
    ServerDispatch.updateStatus(state, description);
  }

  updateUser(user: Data.ServerInfo_User): void {
    ServerDispatch.updateUser(user);
  }

  updateUsers(users: Data.ServerInfo_User[]): void {
    ServerDispatch.updateUsers(users);
  }

  userJoined(user: Data.ServerInfo_User): void {
    ServerDispatch.userJoined(user);
  }

  userLeft(userName: string): void {
    ServerDispatch.userLeft(userName);
  }

  serverMessage(message: string): void {
    ServerDispatch.serverMessage(message);
  }

  accountAwaitingActivation(options: PendingActivation): void {
    ServerDispatch.accountAwaitingActivation(options);
  }

  accountActivationSuccess(): void {
    ServerDispatch.accountActivationSuccess();
  }

  accountActivationFailed(): void {
    ServerDispatch.accountActivationFailed();
  }

  registrationRequiresEmail(): void {
    ServerDispatch.registrationRequiresEmail();
  }

  registrationSuccess(): void {
    ServerDispatch.registrationSuccess();
  }

  registrationFailed(reason: string, endTime?: number): void {
    ServerDispatch.registrationFailed(reason, endTime);
  }

  registrationEmailError(error: string): void {
    ServerDispatch.registrationEmailError(error);
  }

  registrationPasswordError(error: string): void {
    ServerDispatch.registrationPasswordError(error);
  }

  registrationUserNameError(error: string): void {
    ServerDispatch.registrationUserNameError(error);
  }

  resetPasswordChallenge(): void {
    ServerDispatch.resetPasswordChallenge();
  }

  resetPassword(): void {
    ServerDispatch.resetPassword();
  }

  resetPasswordSuccess(): void {
    ServerDispatch.resetPasswordSuccess();
  }

  resetPasswordFailed(): void {
    ServerDispatch.resetPasswordFailed();
  }

  accountPasswordChange(): void {
    ServerDispatch.accountPasswordChange();
  }

  accountEditChanged(realName?: string, email?: string, country?: string): void {
    ServerDispatch.accountEditChanged({ realName, email, country });
  }

  accountImageChanged(avatarBmp: Uint8Array): void {
    ServerDispatch.accountImageChanged({ avatarBmp });
  }

  getUserInfo(userInfo: Data.ServerInfo_User): void {
    ServerDispatch.getUserInfo(userInfo);
  }

  getGamesOfUser(userName: string, response: Data.Response_GetGamesOfUser): void {
    ServerDispatch.gamesOfUser(userName, response);
  }

  gameJoined(gameJoinedData: Data.Event_GameJoined): void {
    GameDispatch.gameJoined(gameJoinedData);
  }

  notifyUser(notification: Data.Event_NotifyUser): void {
    ServerDispatch.notifyUser(notification);
  }

  playerPropertiesChanged(gameId: number, playerId: number, payload: Data.Event_PlayerPropertiesChanged): void {
    if (payload.playerProperties) {
      GameDispatch.playerPropertiesChanged(gameId, playerId, payload.playerProperties);
    }
  }

  serverShutdown(data: Data.Event_ServerShutdown): void {
    ServerDispatch.serverShutdown(data);
  }

  userMessage(messageData: Data.Event_UserMessage): void {
    ServerDispatch.userMessage(messageData);
  }

  addToList(list: string, userName: string): void {
    ServerDispatch.addToList(list, userName);
  }

  removeFromList(list: string, userName: string): void {
    ServerDispatch.removeFromList(list, userName);
  }

  deleteServerDeck(deckId: number): void {
    ServerDispatch.deckDelete(deckId);
  }

  updateServerDecks(deckList: Data.Response_DeckList): void {
    ServerDispatch.backendDecks(deckList);
  }

  uploadServerDeck(path: string, treeItem: Data.ServerInfo_DeckStorage_TreeItem): void {
    ServerDispatch.deckUpload(path, treeItem);
  }

  createServerDeckDir(path: string, dirName: string): void {
    ServerDispatch.deckNewDir(path, dirName);
  }

  deleteServerDeckDir(path: string): void {
    ServerDispatch.deckDelDir(path);
  }

  replayList(matchList: Data.ServerInfo_ReplayMatch[]): void {
    ServerDispatch.replayList(matchList);
  }

  replayAdded(matchInfo: Data.ServerInfo_ReplayMatch): void {
    ServerDispatch.replayAdded(matchInfo);
  }

  replayModifyMatch(gameId: number, doNotHide: boolean): void {
    ServerDispatch.replayModifyMatch(gameId, doNotHide);
  }

  replayDeleteMatch(gameId: number): void {
    ServerDispatch.replayDeleteMatch(gameId);
  }

  downloadServerDeck(deckId: number, response: Data.Response_DeckDownload): void {
    ServerDispatch.deckDownloaded(deckId, response.deck);
  }

  replayDownloaded(replayId: number, response: Data.Response_ReplayDownload): void {
    ServerDispatch.replayDownloaded(replayId, response.replayData);
  }
}
