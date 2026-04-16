import { App } from '@app/types';
import {
  WebClient,
  StatusEnum,
  SessionEvents,
  RoomEvents,
  GameEvents,
  SessionCommands,
  generateSalt,
  passwordSaltSupported,
} from '@app/websocket';
import type { WebClientConfig } from '@app/websocket';

import { createWebClientResponse } from './response';
import { consumePendingOptions } from './connectionState';
import { PROTOCOL_VERSION } from './config';

export function initWebClient(): void {
  const response = createWebClientResponse();

  const config: WebClientConfig = {
    response,

    onServerIdentified: (info) => {
      const { serverName, serverVersion, protocolVersion, serverOptions } = info;
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
        case App.WebSocketConnectReason.LOGIN: {
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
        case App.WebSocketConnectReason.REGISTER: {
          const { password, ...rest } = options;
          const passwordSalt = getPasswordSalt ? generateSalt() : null;
          SessionCommands.register(rest, password, passwordSalt);
          break;
        }
        case App.WebSocketConnectReason.ACTIVATE_ACCOUNT: {
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
        case App.WebSocketConnectReason.PASSWORD_RESET_REQUEST:
          SessionCommands.forgotPasswordRequest(options);
          break;
        case App.WebSocketConnectReason.PASSWORD_RESET_CHALLENGE:
          SessionCommands.forgotPasswordChallenge(options);
          break;
        case App.WebSocketConnectReason.PASSWORD_RESET: {
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
          SessionCommands.updateStatus(StatusEnum.DISCONNECTED, `Unknown Connection Reason: ${options.reason}`);
          SessionCommands.disconnect();
          break;
        }
      }

      response.session.updateInfo(serverName, serverVersion);
    },

    sessionEvents: SessionEvents,
    roomEvents: RoomEvents,
    gameEvents: GameEvents,
    keepAliveFn: (cb) => SessionCommands.ping(cb),
  };

  new WebClient(config);
}
