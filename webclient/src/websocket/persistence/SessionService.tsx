import { ServerDispatch, ServerConnectParams } from "store";
import { StatusEnum } from "types";

import { sanitizeHtml } from "websocket/utils";
import { WebClient } from "websocket/WebClient";

import { NormalizeService } from "websocket";

export default class SessionService {
  webClient: WebClient;

  constructor(webClient) {
    this.webClient = webClient;
  }

  clearStore() {
    ServerDispatch.clearStore();
  }

  connectServer(options: ServerConnectParams) {
    ServerDispatch.connectServer();
    this.webClient.updateStatus(StatusEnum.CONNECTING, "Connecting...");
    this.webClient.connect(options);
  }

  disconnectServer() {
    this.webClient.updateStatus(StatusEnum.DISCONNECTING, "Disconnecting...");
    this.webClient.disconnect();
  }

  connectionClosed(reason) {
    ServerDispatch.connectionClosed(reason);
  }

  updateBuddyList(buddyList) {
    ServerDispatch.updateBuddyList(buddyList);
  }

  addToBuddyList(user) {
    ServerDispatch.addToBuddyList(user);
  }

  removeFromBuddyList(userName) {
    ServerDispatch.removeFromBuddyList(userName);
  }

  updateIgnoreList(ignoreList) {
    ServerDispatch.updateIgnoreList(ignoreList);
  }

  addToIgnoreList(user) {
    ServerDispatch.addToIgnoreList(user);
  }

  removeFromIgnoreList(userName) {
    ServerDispatch.removeFromIgnoreList(userName);
  }

  updateInfo(name, version) {
    ServerDispatch.updateInfo(name, version);
  }

  updateStatus(state, description) {
    ServerDispatch.updateStatus(state, description);

    if (state === StatusEnum.DISCONNECTED) {
      this.connectionClosed({ reason: description });
    }
  }

  updateUser(user) {
    ServerDispatch.updateUser(user);
  }

  updateUsers(users) {
    ServerDispatch.updateUsers(users);
  }

  userJoined(user) {
    ServerDispatch.userJoined(user);
  }

  userLeft(userId) {
    ServerDispatch.userLeft(userId);
  }

  viewLogs(logs) {
    ServerDispatch.viewLogs(NormalizeService.normalizeLogs(logs));
  }

  serverMessage(message) {
    ServerDispatch.serverMessage(sanitizeHtml(message));
  }
}
