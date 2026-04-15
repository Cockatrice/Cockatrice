import { App, Data, Enriched } from '@app/types';

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
import { SessionPersistence } from '../../persistence';

export function serverIdentification(info: Data.Event_ServerIdentification): void {
  const { serverName, serverVersion, protocolVersion, serverOptions } = info;
  if (protocolVersion !== PROTOCOL_VERSION) {
    updateStatus(App.StatusEnum.DISCONNECTED, `Protocol version mismatch: ${protocolVersion}`);
    disconnect();
    return;
  }

  const getPasswordSalt = passwordSaltSupported(serverOptions);
  const options = webClient.options;

  if (!options) {
    updateStatus(App.StatusEnum.DISCONNECTED, 'Missing connection options');
    disconnect();
    return;
  }

  // Strip credentials before handing off to session commands — they travel as
  // separate function args so they can't accidentally ride along in the
  // typed options object that flows downstream.
  switch (options.reason) {
    case App.WebSocketConnectReason.LOGIN: {
      const { password, ...rest } = options;
      updateStatus(App.StatusEnum.LOGGING_IN, 'Logging In...');
      if (getPasswordSalt) {
        requestPasswordSalt(rest, password);
      } else {
        login(rest, password);
      }
      break;
    }
    case App.WebSocketConnectReason.REGISTER: {
      const { password, ...rest } = options;
      const passwordSalt = getPasswordSalt ? generateSalt() : null;
      register(rest, password, passwordSalt);
      break;
    }
    case App.WebSocketConnectReason.ACTIVATE_ACCOUNT: {
      const { password, ...rest } = options;
      if (getPasswordSalt) {
        requestPasswordSalt(rest, password);
      } else {
        activate(rest, password);
      }
      break;
    }
    case App.WebSocketConnectReason.PASSWORD_RESET_REQUEST:
      forgotPasswordRequest(options);
      break;
    case App.WebSocketConnectReason.PASSWORD_RESET_CHALLENGE:
      forgotPasswordChallenge(options);
      break;
    case App.WebSocketConnectReason.PASSWORD_RESET: {
      const { newPassword, ...rest } = options;
      if (getPasswordSalt) {
        requestPasswordSalt(rest, undefined, newPassword);
      } else {
        forgotPasswordReset(rest, newPassword);
      }
      break;
    }
    default: {
      const { reason } = options as Enriched.WebSocketConnectOptions;
      updateStatus(App.StatusEnum.DISCONNECTED, `Unknown Connection Reason: ${reason}`);
      disconnect();
      break;
    }
  }

  webClient.options = null;
  SessionPersistence.updateInfo(serverName, serverVersion);
}
