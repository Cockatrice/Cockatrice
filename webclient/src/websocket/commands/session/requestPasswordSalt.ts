import { RequestPasswordSaltParams } from 'store';
import { StatusEnum, WebSocketConnectOptions, WebSocketConnectReason } from 'types';

import webClient from '../../WebClient';
import { SessionPersistence } from '../../persistence';

import {
  activateAccount,
  disconnect,
  login,
  resetPassword,
  updateStatus
} from './';

export function requestPasswordSalt(options: WebSocketConnectOptions): void {
  const { userName } = options as RequestPasswordSaltParams;

  const registerConfig = {
    ...webClient.clientConfig,
    userName,
  };

  const CmdRequestPasswordSalt = webClient.protobuf.controller.Command_RequestPasswordSalt.create(registerConfig);

  const sc = webClient.protobuf.controller.SessionCommand.create({
    '.Command_RequestPasswordSalt.ext': CmdRequestPasswordSalt
  });

  webClient.protobuf.sendSessionCommand(sc, raw => {
    switch (raw.responseCode) {
      case webClient.protobuf.controller.Response.ResponseCode.RespOk: {
        const passwordSalt = raw['.Response_PasswordSalt.ext']?.passwordSalt;

        switch (options.reason) {
          case WebSocketConnectReason.ACTIVATE_ACCOUNT: {
            activateAccount(options, passwordSalt);
            break;
          }

          case WebSocketConnectReason.PASSWORD_RESET: {
            resetPassword(options, passwordSalt);
            break;
          }

          case WebSocketConnectReason.LOGIN:
          default: {
            login(options, passwordSalt);
          }
        }

        return;
      }
      case webClient.protobuf.controller.Response.ResponseCode.RespRegistrationRequired: {
        updateStatus(StatusEnum.DISCONNECTED, 'Login failed: registration required');
        break;
      }
      default: {
        updateStatus(StatusEnum.DISCONNECTED, 'Login failed: Unknown Reason');
      }
    }

    switch (options.reason) {
      case WebSocketConnectReason.ACTIVATE_ACCOUNT: {
        SessionPersistence.accountActivationFailed();
        break;
      }

      case WebSocketConnectReason.PASSWORD_RESET: {
        SessionPersistence.resetPasswordFailed();
        break;
      }

      case WebSocketConnectReason.LOGIN:
      default: {
        SessionPersistence.loginFailed();
      }
    }

    disconnect();
  });
}
