import { ServerDispatch } from "store";
import { Log, StatusEnum, User } from "types";

import { sanitizeHtml } from "websocket/utils";
import NormalizeService from "../utils/NormalizeService";

export class SessionPersistence {
  static clearStore() {
    ServerDispatch.clearStore();
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
    console.log("Open Modal for Activation Code input");
  }

  static accountActivationFailed() {
    console.log("Account activation failed, show an action here");
  }

  static resetPasswordChallenge() {
    console.log("Open Modal asking for Email address associated with account");
  }

  static resetPassword() {
    console.log("Open Modal asking for reset token & new password");

  }

  static resetPasswordSuccess() {
    console.log("User password successfully changed Alert!");
  }

  static resetPasswordFailed() {
    console.log("Open Alert telling user their password request failed for some reason");
  }
}
