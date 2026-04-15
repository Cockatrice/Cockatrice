import { StatusEnum, WebSocketConnectOptions, WebSocketConnectReason } from 'types';

import webClient from '../../WebClient';
import {
  activate,
  disconnect,
  login,
  register,
  requestPasswordSalt,
  forgotPasswordChallenge,
  forgotPasswordRequest,
  forgotPasswordReset,
  updateStatus,
} from '../../commands/session';
import { generateSalt, passwordSaltSupported } from '../../utils';
import { ServerIdentificationData } from './interfaces';
import { SessionPersistence } from '../../persistence';

export function serverIdentification(info: ServerIdentificationData): void {
  const { serverName, serverVersion, protocolVersion, serverOptions } = info;
  if (protocolVersion !== webClient.protocolVersion) {
    updateStatus(StatusEnum.DISCONNECTED, `Protocol version mismatch: ${protocolVersion}`);
    disconnect();
    return;
  }

  const getPasswordSalt = passwordSaltSupported(serverOptions);
  const connectOptions = { ...webClient.options };

  switch (connectOptions.reason) {
    case WebSocketConnectReason.LOGIN:
      updateStatus(StatusEnum.LOGGING_IN, 'Logging In...');
      if (getPasswordSalt) {
        requestPasswordSalt(connectOptions);
      } else {
        login(connectOptions);
      }
      break;
    case WebSocketConnectReason.REGISTER:
      const passwordSalt = getPasswordSalt ? generateSalt() : null;
      register(connectOptions, passwordSalt);
      break;
    case WebSocketConnectReason.ACTIVATE_ACCOUNT:
      if (getPasswordSalt) {
        requestPasswordSalt(connectOptions);
      } else {
        activate(connectOptions);
      }
      break;
    case WebSocketConnectReason.PASSWORD_RESET_REQUEST:
      forgotPasswordRequest(connectOptions);
      break;
    case WebSocketConnectReason.PASSWORD_RESET_CHALLENGE:
      forgotPasswordChallenge(connectOptions);
      break;
    case WebSocketConnectReason.PASSWORD_RESET:
      if (getPasswordSalt) {
        requestPasswordSalt(connectOptions);
      } else {
        forgotPasswordReset(connectOptions);
      }
      break;
    default:
      updateStatus(StatusEnum.DISCONNECTED, 'Unknown Connection Reason: ' + connectOptions.reason);
      disconnect();
      break;
  }

  webClient.options = {} as WebSocketConnectOptions;
  SessionPersistence.updateInfo(serverName, serverVersion);
}
