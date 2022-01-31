import { ServerDispatch } from 'store';
import { Log, StatusEnum, User, WebSocketConnectOptions } from 'types';

import { sanitizeHtml } from 'websocket/utils';
import NormalizeService from '../utils/NormalizeService';

export class SessionPersistence {
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

  static viewLogs(logs: Log[]) {
    ServerDispatch.viewLogs(NormalizeService.normalizeLogs(logs));
  }

  static serverMessage(message: string) {
    ServerDispatch.serverMessage(sanitizeHtml(message));
  }

  static accountAwaitingActivation() {
    ServerDispatch.accountAwaitingActivation();
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

  static registrationFailed(error: string) {
    ServerDispatch.registrationFailed(error);
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
}
