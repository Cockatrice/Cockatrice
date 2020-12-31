import { Dispatch, ServerConnectParams } from "store/server";
import { StatusEnum } from "types";

import { sanitizeHtml } from "../util";
import { WebClient } from "../WebClient";

import { NormalizeService } from "./NormalizeService";

export class SessionService {
  webClient: WebClient;

  constructor(webClient) {
    this.webClient = webClient;
  }

  clearStore() {
    Dispatch.clearStore();
  }

  connectServer(options: ServerConnectParams) {
    Dispatch.connectServer();
    this.webClient.updateStatus(StatusEnum.CONNECTING, "Connecting...");
    this.webClient.connect(options);
  }

  disconnectServer() {
    this.webClient.updateStatus(StatusEnum.DISCONNECTING, "Disconnecting...");
    this.webClient.disconnect();
  }

  connectionClosed(reason) {
    Dispatch.connectionClosed(reason);
  }

  updateBuddyList(buddyList) {
    Dispatch.updateBuddyList(buddyList);
  }

  addToBuddyList(user) {
    Dispatch.addToBuddyList(user);
  }

  removeFromBuddyList(userName) {
    Dispatch.removeFromBuddyList(userName);
  }

  updateIgnoreList(ignoreList) {
    Dispatch.updateIgnoreList(ignoreList);
  }

  addToIgnoreList(user) {
    Dispatch.addToIgnoreList(user);
  }

  removeFromIgnoreList(userName) {
    Dispatch.removeFromIgnoreList(userName);
  }

  updateInfo(name, version) {
    Dispatch.updateInfo(name, version);
  }

  updateStatus(state, description) {
    Dispatch.updateStatus(state, description);

    if (state === StatusEnum.DISCONNECTED) {
      this.connectionClosed({ reason: description });
    }
  }

  updateUser(user) {
    Dispatch.updateUser(user);
  }

  updateUsers(users) {
    Dispatch.updateUsers(users);
  }

  userJoined(user) {
    Dispatch.userJoined(user);
  }

  userLeft(userId) {
    Dispatch.userLeft(userId);
  }

  viewLogs(logs) {
    Dispatch.viewLogs(NormalizeService.normalizeLogs(logs));
  }

  serverMessage(message) {
    Dispatch.serverMessage(sanitizeHtml(message));
  }
}
