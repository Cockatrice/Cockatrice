import type { Event_ServerIdentification } from '@app/generated';
import { WebClient } from '../../WebClient';
import { StatusEnum } from '../../interfaces/StatusEnum';
import { PROTOCOL_VERSION } from '../../config';
import { consumePendingOptions } from '../../utils/connectionState';
import { WebSocketConnectReason } from '../../interfaces/ConnectOptions';
import { generateSalt, passwordSaltSupported } from '../../utils';
import * as SessionCommands from '../../commands/session';

export function serverIdentification(info: Event_ServerIdentification): void {
  const { serverName, serverVersion, protocolVersion, serverOptions } = info;
  const response = WebClient.instance.response;

  if (protocolVersion !== PROTOCOL_VERSION) {
    SessionCommands.updateStatus(StatusEnum.DISCONNECTED, `Protocol version mismatch: ${protocolVersion}`);
    SessionCommands.disconnect();
    return;
  }

  const getPasswordSalt = passwordSaltSupported(serverOptions);
  const options = consumePendingOptions();

  if (!options) {
    SessionCommands.updateStatus(StatusEnum.DISCONNECTED, 'Missing connection options');
    SessionCommands.disconnect();
    return;
  }

  switch (options.reason) {
    case WebSocketConnectReason.LOGIN: {
      const { password, ...rest } = options;
      SessionCommands.updateStatus(StatusEnum.LOGGING_IN, 'Logging In...');
      if (getPasswordSalt) {
        SessionCommands.requestPasswordSalt(rest,
          (salt) => SessionCommands.login(rest, password, salt),
          () => {
            response.session.loginFailed(); SessionCommands.disconnect();
          },
        );
      } else {
        SessionCommands.login(rest, password);
      }
      break;
    }
    case WebSocketConnectReason.REGISTER: {
      const { password, ...rest } = options;
      const passwordSalt = getPasswordSalt ? generateSalt() : null;
      SessionCommands.register(rest, password, passwordSalt);
      break;
    }
    case WebSocketConnectReason.ACTIVATE_ACCOUNT: {
      const { password, ...rest } = options;
      if (getPasswordSalt) {
        SessionCommands.requestPasswordSalt(rest,
          (salt) => SessionCommands.activate(rest, password, salt),
          () => {
            response.session.accountActivationFailed(); SessionCommands.disconnect();
          },
        );
      } else {
        SessionCommands.activate(rest, password);
      }
      break;
    }
    case WebSocketConnectReason.PASSWORD_RESET_REQUEST:
      SessionCommands.forgotPasswordRequest(options);
      break;
    case WebSocketConnectReason.PASSWORD_RESET_CHALLENGE:
      SessionCommands.forgotPasswordChallenge(options);
      break;
    case WebSocketConnectReason.PASSWORD_RESET: {
      const { newPassword, ...rest } = options;
      if (getPasswordSalt) {
        SessionCommands.requestPasswordSalt(rest,
          (salt) => SessionCommands.forgotPasswordReset(rest, newPassword, salt),
          () => {
            response.session.resetPasswordFailed(); SessionCommands.disconnect();
          },
        );
      } else {
        SessionCommands.forgotPasswordReset(rest, newPassword);
      }
      break;
    }
    default: {
      SessionCommands.updateStatus(StatusEnum.DISCONNECTED, `Unknown Connection Reason: ${(options as { reason: number }).reason}`);
      SessionCommands.disconnect();
      break;
    }
  }

  response.session.updateInfo(serverName, serverVersion);
}
