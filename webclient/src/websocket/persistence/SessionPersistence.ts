import { ServerDispatch } from 'store';
import { DeckStorageTreeItem, StatusEnum, User, WebSocketConnectOptions } from 'types';

import { sanitizeHtml } from 'websocket/utils';
import {
  GameJoinedData,
  NotifyUserData,
  PlayerGamePropertiesData,
  ServerShutdownData,
  UserMessageData
} from '../events/session/interfaces';
import NormalizeService from '../utils/NormalizeService';
import { DeckList } from 'types';
import { common } from 'protobufjs';
import IBytesValue = common.IBytesValue;

export class SessionPersistence {
  static initialized() {
    ServerDispatch.initialized();
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

  static updateBuddyList(buddyList) {
    ServerDispatch.updateBuddyList(buddyList);
  }

  static addToBuddyList(user: User) {
    ServerDispatch.addToBuddyList(user);
  }

  static removeFromBuddyList(userName: string) {
    ServerDispatch.removeFromBuddyList(userName);
  }

  static updateIgnoreList(ignoreList) {
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
    const reasonMsg = endTime ? NormalizeService.normalizeBannedUserError(reason, endTime) : reason;

    ServerDispatch.registrationFailed(reasonMsg);
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

  static accountImageChanged(avatarBmp: IBytesValue): void {
    ServerDispatch.accountImageChanged({ avatarBmp });
  }

  static directMessageSent(userName: string, message: string): void {
    ServerDispatch.directMessageSent(userName, message);
  }

  static getUserInfo(userInfo: User) {
    ServerDispatch.getUserInfo(userInfo);
  }

  static getGamesOfUser(userName: string, response: any): void {
    console.log('getGamesOfUser');
  }

  static gameJoined(gameJoinedData: GameJoinedData): void {
    console.log('gameJoined', gameJoinedData);
  }

  static notifyUser(notification: NotifyUserData): void {
    ServerDispatch.notifyUser(notification);
  }

  static playerPropertiesChanged(payload: PlayerGamePropertiesData): void {
    console.log('playerPropertiesChanged', payload);
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

  static deckDelete(deckId: number): void {
    console.log('deckDelete', deckId);
  }

  static deckDeleteDir(path: string): void {
    console.log('deckDeleteDir', path);
  }

  static deckDownload(deckId: number): void {
    console.log('deckDownload', deckId);
  }

  static deckList(deckList: DeckList): void {
    console.log('deckList', deckList);
  }

  static deckNewDir(path: string, dirName: string): void {
    console.log('deckNewDir', path, dirName);
  }

  static deckUpload(treeItem: DeckStorageTreeItem): void {
    console.log('deckUpload', treeItem);
  }
}


