import { StatusEnum, WebSocketConnectOptions, WebSocketConnectReason } from 'types';
import webClient from '../../WebClient';
import { updateStatus } from './';

export function connect(options: WebSocketConnectOptions, reason: WebSocketConnectReason): void {
  switch (reason) {
    case WebSocketConnectReason.LOGIN:
    case WebSocketConnectReason.REGISTER:
    case WebSocketConnectReason.ACTIVATE_ACCOUNT:
    case WebSocketConnectReason.PASSWORD_RESET_REQUEST:
    case WebSocketConnectReason.PASSWORD_RESET_CHALLENGE:
    case WebSocketConnectReason.PASSWORD_RESET:
      updateStatus(StatusEnum.CONNECTING, 'Connecting...');
      break;
    case WebSocketConnectReason.TEST_CONNECTION:
      webClient.testConnect({ ...options });
      return;
    default:
      updateStatus(StatusEnum.DISCONNECTED, 'Unknown Connection Attempt: ' + reason);
      return;
  }

  webClient.connect({ ...options, reason });
}
