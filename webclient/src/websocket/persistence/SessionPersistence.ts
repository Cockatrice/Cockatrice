import { GameDispatch, ServerDispatch } from '@app/store';
import { App, Data, Enriched } from '@app/types';
import { sanitizeHtml } from '../utils';

export class SessionPersistence {
  static initialized() {
    ServerDispatch.initialized();
  }

  static connectionAttempted() {
    ServerDispatch.connectionAttempted();
  }

  static clearStore() {
    ServerDispatch.clearStore();
  }

  static loginSuccessful(options: Enriched.LoginSuccessContext) {
    ServerDispatch.loginSuccessful(options);
  }

  static loginFailed() {
    ServerDispatch.loginFailed();
  }

  static connectionFailed() {
    ServerDispatch.connectionFailed();
  }

  static testConnectionSuccessful() {
    ServerDispatch.testConnectionSuccessful();
  }

  static testConnectionFailed() {
    ServerDispatch.testConnectionFailed();
  }

  static updateBuddyList(buddyList: Data.ServerInfo_User[]) {
    ServerDispatch.updateBuddyList(buddyList);
  }

  static addToBuddyList(user: Data.ServerInfo_User) {
    ServerDispatch.addToBuddyList(user);
  }

  static removeFromBuddyList(userName: string) {
    ServerDispatch.removeFromBuddyList(userName);
  }

  static updateIgnoreList(ignoreList: Data.ServerInfo_User[]) {
    ServerDispatch.updateIgnoreList(ignoreList);
  }

  static addToIgnoreList(user: Data.ServerInfo_User) {
    ServerDispatch.addToIgnoreList(user);
  }

  static removeFromIgnoreList(userName: string) {
    ServerDispatch.removeFromIgnoreList(userName);
  }

  static updateInfo(name: string, version: string) {
    ServerDispatch.updateInfo(name, version);
  }

  static updateStatus(state: App.StatusEnum, description: string) {
    ServerDispatch.updateStatus(state, description);
  }

  static updateUser(user: Data.ServerInfo_User) {
    ServerDispatch.updateUser(user);
  }

  static updateUsers(users: Data.ServerInfo_User[]) {
    ServerDispatch.updateUsers(users);
  }

  static userJoined(user: Data.ServerInfo_User) {
    ServerDispatch.userJoined(user);
  }

  static userLeft(userName: string) {
    ServerDispatch.userLeft(userName);
  }

  static serverMessage(message: string) {
    ServerDispatch.serverMessage(sanitizeHtml(message));
  }

  static accountAwaitingActivation(options: Enriched.PendingActivationContext) {
    ServerDispatch.accountAwaitingActivation(options);
  }

  static accountActivationSuccess() {
    ServerDispatch.accountActivationSuccess();
  }

  static accountActivationFailed() {
    ServerDispatch.accountActivationFailed();
  }

  static registrationRequiresEmail() {
    ServerDispatch.registrationRequiresEmail();
  }

  static registrationSuccess() {
    ServerDispatch.registrationSuccess();
  }

  static registrationFailed(reason: string, endTime?: number) {
    ServerDispatch.registrationFailed(reason, endTime);
  }

  static registrationEmailError(error: string) {
    ServerDispatch.registrationEmailError(error);
  }

  static registrationPasswordError(error: string) {
    ServerDispatch.registrationPasswordError(error);
  }

  static registrationUserNameError(error: string) {
    ServerDispatch.registrationUserNameError(error);
  }

  static resetPasswordChallenge() {
    ServerDispatch.resetPasswordChallenge();
  }

  static resetPassword() {
    ServerDispatch.resetPassword();
  }

  static resetPasswordSuccess() {
    ServerDispatch.resetPasswordSuccess();
  }

  static resetPasswordFailed() {
    ServerDispatch.resetPasswordFailed();
  }

  static accountPasswordChange(): void {
    ServerDispatch.accountPasswordChange();
  }

  static accountEditChanged(realName?: string, email?: string, country?: string): void {
    ServerDispatch.accountEditChanged({ realName, email, country });
  }

  static accountImageChanged(avatarBmp: Uint8Array): void {
    ServerDispatch.accountImageChanged({ avatarBmp });
  }

  static getUserInfo(userInfo: Data.ServerInfo_User) {
    ServerDispatch.getUserInfo(userInfo);
  }

  static getGamesOfUser(userName: string, response: Data.Response_GetGamesOfUser): void {
    ServerDispatch.gamesOfUser(userName, response);
  }

  static gameJoined(gameJoinedData: Data.Event_GameJoined): void {
    GameDispatch.gameJoined(gameJoinedData);
  }

  static notifyUser(notification: Data.Event_NotifyUser): void {
    ServerDispatch.notifyUser(notification);
  }

  static playerPropertiesChanged(gameId: number, playerId: number, payload: Data.Event_PlayerPropertiesChanged): void {
    if (payload.playerProperties) {
      GameDispatch.playerPropertiesChanged(gameId, playerId, payload.playerProperties);
    }
  }

  static serverShutdown(data: Data.Event_ServerShutdown): void {
    ServerDispatch.serverShutdown(data);
  }

  static userMessage(messageData: Data.Event_UserMessage): void {
    ServerDispatch.userMessage(messageData);
  }

  static addToList(list: string, userName: string): void {
    ServerDispatch.addToList(list, userName)
  }

  static removeFromList(list: string, userName: string): void {
    ServerDispatch.removeFromList(list, userName);
  }

  static deleteServerDeck(deckId: number): void {
    ServerDispatch.deckDelete(deckId);
  }

  static updateServerDecks(deckList: Data.Response_DeckList): void {
    ServerDispatch.backendDecks(deckList);
  }

  static uploadServerDeck(path: string, treeItem: Data.ServerInfo_DeckStorage_TreeItem): void {
    ServerDispatch.deckUpload(path, treeItem);
  }

  static createServerDeckDir(path: string, dirName: string): void {
    ServerDispatch.deckNewDir(path, dirName);
  }

  static deleteServerDeckDir(path: string): void {
    ServerDispatch.deckDelDir(path);
  }

  static replayList(matchList: Data.ServerInfo_ReplayMatch[]): void {
    ServerDispatch.replayList(matchList);
  }

  static replayAdded(matchInfo: Data.ServerInfo_ReplayMatch): void {
    ServerDispatch.replayAdded(matchInfo);
  }

  static replayModifyMatch(gameId: number, doNotHide: boolean): void {
    ServerDispatch.replayModifyMatch(gameId, doNotHide);
  }

  static replayDeleteMatch(gameId: number): void {
    ServerDispatch.replayDeleteMatch(gameId);
  }
}


