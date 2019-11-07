import { store } from 'store';
import { StatusEnum } from 'common/types';
import { Actions, Selectors, ServerConnectParams } from 'store/server';

import WebClient from 'WebClient/WebClient';

export class ServerService {
  static connectServer(options: ServerConnectParams) {
     ServerService.updateStatus(StatusEnum.CONNECTING, 'Connecting');
     store.dispatch(Actions.connectServer());

     WebClient.getInstance().connect(options, {
      onopen: () => {
        ServerService.updateStatus(StatusEnum.CONNECTED, 'Connected');
      },
      onclose: () => {
        const reason = 'Connection Closed';
        ServerService.connectionClosed(reason);
        ServerService.updateStatus(StatusEnum.DISCONNECTED, reason);
      },
      onerror: () => {
        const reason = 'Connection Failed';
        ServerService.connectionClosed(reason);
        ServerService.updateStatus(StatusEnum.DISCONNECTED, reason);
      },
    });
  }

  static connectionClosed(reason) {
    store.dispatch(Actions.connectionClosed(reason));
  }

  static getStatus() {
    return Selectors.getStatus(store.getState());
  }

  static updateStatus(state, description) {
    store.dispatch(Actions.updateStatus({
      state,
      description
    }));

    if (state === StatusEnum.DISCONNECTED) {
      store.dispatch(Actions.connectionClosed({
        reason: description
      }));
    }
  }

  static serverMessage(message) {
    store.dispatch(Actions.serverMessage(message));
  }
}
