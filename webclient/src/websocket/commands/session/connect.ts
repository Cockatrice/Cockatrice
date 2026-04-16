import { App, Enriched } from '@app/types';
import { WebClient } from '../../WebClient';
import { updateStatus } from './';

export function connect(options: Enriched.WebSocketConnectOptions): void {
  switch (options.reason) {
    case App.WebSocketConnectReason.LOGIN:
    case App.WebSocketConnectReason.REGISTER:
    case App.WebSocketConnectReason.ACTIVATE_ACCOUNT:
    case App.WebSocketConnectReason.PASSWORD_RESET_REQUEST:
    case App.WebSocketConnectReason.PASSWORD_RESET_CHALLENGE:
    case App.WebSocketConnectReason.PASSWORD_RESET:
      updateStatus(App.StatusEnum.CONNECTING, 'Connecting...');
      WebClient.instance.connect(options);
      return;
    case App.WebSocketConnectReason.TEST_CONNECTION:
      WebClient.instance.testConnect(options);
      return;
    default: {
      const { reason } = options as Enriched.WebSocketConnectOptions;
      updateStatus(App.StatusEnum.DISCONNECTED, `Unknown Connection Attempt: ${reason}`);
      return;
    }
  }
}
