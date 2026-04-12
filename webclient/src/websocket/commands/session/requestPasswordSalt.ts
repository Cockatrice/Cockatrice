import { RequestPasswordSaltParams } from 'store';
import { StatusEnum, WebSocketConnectOptions, WebSocketConnectReason } from 'types';

import webClient from '../../WebClient';
import { BackendService } from '../../services/BackendService';
import { ProtoController } from '../../services/ProtoController';
import { SessionPersistence } from '../../persistence';

import {
  activate,
  disconnect,
  login,
  forgotPasswordReset,
  updateStatus
} from './';

export function requestPasswordSalt(options: WebSocketConnectOptions, password?: string, newPassword?: string): void {
  const { userName } = options as RequestPasswordSaltParams;

  const onFailure = () => {
    switch (options.reason) {
      case WebSocketConnectReason.ACTIVATE_ACCOUNT:
        SessionPersistence.accountActivationFailed();
        break;
      case WebSocketConnectReason.PASSWORD_RESET:
        SessionPersistence.resetPasswordFailed();
        break;
      default:
        SessionPersistence.loginFailed();
    }
    disconnect();
  };

  BackendService.sendSessionCommand('Command_RequestPasswordSalt', {
    ...webClient.clientConfig,
    userName,
  }, {
    responseName: 'Response_PasswordSalt',
    onSuccess: (resp) => {
      const passwordSalt = resp?.passwordSalt;

      switch (options.reason) {
        case WebSocketConnectReason.ACTIVATE_ACCOUNT:
          activate(options, password, passwordSalt);
          break;
        case WebSocketConnectReason.PASSWORD_RESET:
          forgotPasswordReset(options, newPassword, passwordSalt);
          break;
        default:
          login(options, password, passwordSalt);
      }
    },
    onResponseCode: {
      [ProtoController.root.Response.ResponseCode.RespRegistrationRequired]: () => {
        updateStatus(StatusEnum.DISCONNECTED, 'Login failed: registration required');
        onFailure();
      },
    },
    onError: () => {
      updateStatus(StatusEnum.DISCONNECTED, 'Login failed: Unknown Reason');
      onFailure();
    },
  });
}
