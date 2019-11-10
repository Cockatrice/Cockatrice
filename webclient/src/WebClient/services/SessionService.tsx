import { Dispatch, ServerConnectParams } from 'store/server';
import { StatusEnum } from 'types';

import { sanitizeHtml } from '../util';
import { WebClient } from '../WebClient';

export class SessionService {
  webClient: WebClient;

  constructor(webClient) {
    this.webClient = webClient;
  }

  // @TODO determine proper connect status tracking
  connectServer(options: ServerConnectParams) {
    Dispatch.connectServer();
    this.webClient.connect(options);
    this.updateStatus(StatusEnum.CONNECTING, 'Connecting');
  }

  fetchRooms() {
    this.webClient.commands.session.fetchRooms();
  }

  joinRoom(roomId: string) {
    this.webClient.commands.session.joinRoom(roomId);
  }

  connectionClosed(reason) {
    Dispatch.connectionClosed(reason);
  }

  updateInfo(name, version) {
    Dispatch.updateInfo(name, version);
  }

  updateStatus(state, description) {
    Dispatch.updateStatus(state, description);

    // @TODO determine if this is desired
    // if (state === StatusEnum.DISCONNECTED) {
    //   Dispatch.connectionClosed({
    //     reason: description
    //   });
    // }
  }

  updateUser(user) {
    Dispatch.updateUser(user);
  }

  serverMessage(message) {
    Dispatch.serverMessage(sanitizeHtml(message));
  }
}
