import { ServerDispatch } from 'store';
import { Log, StatusEnum, User, WebSocketConnectOptions } from 'types';

import { sanitizeHtml } from 'websocket/utils';
import NormalizeService from '../utils/NormalizeService';

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

  static accountPasswordChange(): void {
    console.log('accountPassword');
  }

  static accountEditChanged(realName?: string, email?: string, country?: string): void {
    console.log('accountEditChange');
  }

  static accountImageChanged(): void {
    console.log("accountImageChanged");
  }

  static directMessageSent(userName: string, message: string): void {
    console.log("directMessageSent");
  }
}
