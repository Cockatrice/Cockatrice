import { StatusEnum, WebSocketConnectOptions, WebSocketConnectReason } from 'types';

import webClient from '../../WebClient';
import { PROTOCOL_VERSION } from '../../config';
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
  if (protocolVersion !== PROTOCOL_VERSION) {
    updateStatus(StatusEnum.DISCONNECTED, `Protocol version mismatch: ${protocolVersion}`);
    disconnect();
    return;
  }

  const getPasswordSalt = passwordSaltSupported(serverOptions);
  const { password, newPassword, ...connectOptions } = webClient.options;

  switch (connectOptions.reason) {
    case WebSocketConnectReason.LOGIN:
      updateStatus(StatusEnum.LOGGING_IN, 'Logging In...');
      if (getPasswordSalt) {
        requestPasswordSalt(connectOptions, password);
      } else {
        login(connectOptions, password);
      }
      break;
    case WebSocketConnectReason.REGISTER:
      const passwordSalt = getPasswordSalt ? generateSalt() : null;
      register(connectOptions, password, passwordSalt);
      break;
    case WebSocketConnectReason.ACTIVATE_ACCOUNT:
      if (getPasswordSalt) {
        requestPasswordSalt(connectOptions, password);
      } else {
        activate(connectOptions, password);
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
        requestPasswordSalt(connectOptions, undefined, newPassword);
      } else {
        forgotPasswordReset(connectOptions, newPassword);
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
