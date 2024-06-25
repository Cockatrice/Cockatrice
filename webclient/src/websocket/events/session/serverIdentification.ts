import { StatusEnum, WebSocketConnectReason } from 'types';

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

  const getPasswordSalt = passwordSaltSupported(serverOptions, webClient);
  const { options } = webClient;

  switch (options.reason) {
    case WebSocketConnectReason.LOGIN:
      updateStatus(StatusEnum.LOGGING_IN, 'Logging In...');
      if (getPasswordSalt) {
        requestPasswordSalt(options);
      } else {
        login(options);
      }
      break;
    case WebSocketConnectReason.REGISTER:
      const passwordSalt = getPasswordSalt ? generateSalt() : null;
      register(options, passwordSalt);
      break;
    case WebSocketConnectReason.ACTIVATE_ACCOUNT:
      if (getPasswordSalt) {
        requestPasswordSalt(options);
      } else {
        activate(options);
      }
      break;
    case WebSocketConnectReason.PASSWORD_RESET_REQUEST:
      forgotPasswordRequest(options);
      break;
    case WebSocketConnectReason.PASSWORD_RESET_CHALLENGE:
      forgotPasswordChallenge(options);
      break;
    case WebSocketConnectReason.PASSWORD_RESET:
      if (getPasswordSalt) {
        requestPasswordSalt(options);
      } else {
        forgotPasswordReset(options);
      }
      break;
    default:
      updateStatus(StatusEnum.DISCONNECTED, 'Unknown Connection Reason: ' + options.reason);
      disconnect();
      break;
  }

  webClient.options = {};
  SessionPersistence.updateInfo(serverName, serverVersion);
}
