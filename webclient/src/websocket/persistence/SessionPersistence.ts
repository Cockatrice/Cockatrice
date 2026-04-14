import { GameDispatch, ServerDispatch } from 'store';
import { DeckList, DeckStorageTreeItem, ReplayMatch, StatusEnum, User, WebSocketConnectOptions } from 'types';
import { GameEntry } from 'store/game/game.interfaces';
import { sanitizeHtml } from 'websocket/utils';
import {
  GameJoinedData,
  NotifyUserData,
  PlayerGamePropertiesData,
  ServerShutdownData,
  UserMessageData
} from '../events/session/interfaces';

import type { Response_GetGamesOfUser } from 'generated/proto/response_get_games_of_user_pb';
import type { ServerInfo_Room } from 'generated/proto/serverinfo_room_pb';
import type { ServerInfo_GameType } from 'generated/proto/serverinfo_gametype_pb';

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

  static loginSuccessful(options: WebSocketConnectOptions) {
    ServerDispatch.loginSuccessful(options);
  }

  static loginFailed() {
    ServerDispatch.loginFailed();
  }

  static connectionClosed(reason: number) {
    ServerDispatch.connectionClosed(reason);
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

  static updateBuddyList(buddyList: User[]) {
    ServerDispatch.updateBuddyList(buddyList);
  }

  static addToBuddyList(user: User) {
    ServerDispatch.addToBuddyList(user);
  }

  static removeFromBuddyList(userName: string) {
    ServerDispatch.removeFromBuddyList(userName);
  }

  static updateIgnoreList(ignoreList: User[]) {
    ServerDispatch.updateIgnoreList(ignoreList);
  }

  static addToIgnoreList(user: User) {
    ServerDispatch.addToIgnoreList(user);
  }

  static removeFromIgnoreList(userName: string) {
    ServerDispatch.removeFromIgnoreList(userName);
  }

  static updateInfo(name: string, version: string) {
    ServerDispatch.updateInfo(name, version);
  }

  static updateStatus(state: number, description: string) {
    ServerDispatch.updateStatus(state, description);

    if (state === StatusEnum.DISCONNECTED) {
      this.connectionClosed(state);
    }
  }

  static updateUser(user: User) {
    ServerDispatch.updateUser(user);
  }

  static updateUsers(users: User[]) {
    ServerDispatch.updateUsers(users);
  }

  static userJoined(user: User) {
    ServerDispatch.userJoined(user);
  }

  static userLeft(userName: string) {
    ServerDispatch.userLeft(userName);
  }

  static serverMessage(message: string) {
    ServerDispatch.serverMessage(sanitizeHtml(message));
  }

  static accountAwaitingActivation(options: WebSocketConnectOptions) {
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

  static getUserInfo(userInfo: User) {
    ServerDispatch.getUserInfo(userInfo);
  }

  static getGamesOfUser(userName: string, response: Response_GetGamesOfUser): void {
    const gametypeMap: Record<number, string> = {};
    (response.roomList || []).forEach((room: ServerInfo_Room) => {
      (room.gametypeList || []).forEach((gt: ServerInfo_GameType) => {
        gametypeMap[gt.gameTypeId] = gt.description;
      });
    });
    const games = response.gameList || [];
    ServerDispatch.gamesOfUser(userName, games, gametypeMap);
  }

  static gameJoined(gameJoinedData: GameJoinedData): void {
    const { gameInfo, hostId, playerId, spectator, judge, resuming } = gameJoinedData;
    const gameEntry: GameEntry = {
      gameId: gameInfo.gameId,
      roomId: gameInfo.roomId,
      description: gameInfo.description,
      hostId,
      localPlayerId: playerId,
      spectator,
      judge,
      resuming,
      started: gameInfo.started,
      activePlayerId: -1,
      activePhase: -1,
      secondsElapsed: 0,
      reversed: false,
      players: {},
      messages: [],
    };
    GameDispatch.gameJoined(gameInfo.gameId, gameEntry);
  }

  static notifyUser(notification: NotifyUserData): void {
    ServerDispatch.notifyUser(notification);
  }

  static playerPropertiesChanged(gameId: number, playerId: number, payload: PlayerGamePropertiesData): void {
    if (payload.playerProperties) {
      GameDispatch.playerPropertiesChanged(gameId, playerId, payload.playerProperties);
    }
  }

  static serverShutdown(data: ServerShutdownData): void {
    ServerDispatch.serverShutdown(data);
  }

  static userMessage(messageData: UserMessageData): void {
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

  static updateServerDecks(deckList: DeckList): void {
    ServerDispatch.backendDecks(deckList);
  }

  static uploadServerDeck(path: string, treeItem: DeckStorageTreeItem): void {
    ServerDispatch.deckUpload(path, treeItem);
  }

  static createServerDeckDir(path: string, dirName: string): void {
    ServerDispatch.deckNewDir(path, dirName);
  }

  static deleteServerDeckDir(path: string): void {
    ServerDispatch.deckDelDir(path);
  }

  static replayList(matchList: ReplayMatch[]): void {
    ServerDispatch.replayList(matchList);
  }

  static replayAdded(matchInfo: ReplayMatch): void {
    ServerDispatch.replayAdded(matchInfo);
  }

  static replayModifyMatch(gameId: number, doNotHide: boolean): void {
    ServerDispatch.replayModifyMatch(gameId, doNotHide);
  }

  static replayDeleteMatch(gameId: number): void {
    ServerDispatch.replayDeleteMatch(gameId);
  }
}


