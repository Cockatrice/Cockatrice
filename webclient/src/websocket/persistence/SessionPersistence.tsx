import { ServerDispatch } from "store";
import { StatusEnum } from "types";

import { sanitizeHtml } from "websocket/utils";
import NormalizeService from "../utils/NormalizeService";

export class SessionPersistence {
  static clearStore() {
    ServerDispatch.clearStore();
  }

  static connectionClosed(reason) {
    ServerDispatch.connectionClosed(reason);
  }

  static updateBuddyList(buddyList) {
    ServerDispatch.updateBuddyList(buddyList);
  }

  static addToBuddyList(user) {
    ServerDispatch.addToBuddyList(user);
  }

  static removeFromBuddyList(userName) {
    ServerDispatch.removeFromBuddyList(userName);
  }

  static updateIgnoreList(ignoreList) {
    ServerDispatch.updateIgnoreList(ignoreList);
  }

  static addToIgnoreList(user) {
    ServerDispatch.addToIgnoreList(user);
  }

  static removeFromIgnoreList(userName) {
    ServerDispatch.removeFromIgnoreList(userName);
  }

  static updateInfo(name, version) {
    ServerDispatch.updateInfo(name, version);
  }

  static updateStatus(state, description) {
    ServerDispatch.updateStatus(state, description);

    if (state === StatusEnum.DISCONNECTED) {
      this.connectionClosed({ reason: description });
    }
  }

  static updateUser(user) {
    ServerDispatch.updateUser(user);
  }

  static updateUsers(users) {
    ServerDispatch.updateUsers(users);
  }

  static userJoined(user) {
    ServerDispatch.userJoined(user);
  }

  static userLeft(userId) {
    ServerDispatch.userLeft(userId);
  }

  static viewLogs(logs) {
    ServerDispatch.viewLogs(NormalizeService.normalizeLogs(logs));
  }

  static serverMessage(message) {
    ServerDispatch.serverMessage(sanitizeHtml(message));
  }
}
