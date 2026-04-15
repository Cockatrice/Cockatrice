import { App, Enriched, Data } from '@app/types';

import { create } from '@bufbuild/protobuf';
import { CLIENT_CONFIG } from '../../config';
import webClient from '../../WebClient';

import { SessionPersistence } from '../../persistence';

import {
  activate,
  disconnect,
  login,
  forgotPasswordReset,
  updateStatus
} from './';

type PasswordSaltOptions =
  | Omit<Enriched.LoginConnectOptions, 'password'>
  | Omit<Enriched.ActivateConnectOptions, 'password'>
  | Omit<Enriched.PasswordResetConnectOptions, 'newPassword'>;

export function requestPasswordSalt(options: PasswordSaltOptions, password?: string, newPassword?: string): void {
  const { userName } = options;

  const onFailure = () => {
    switch (options.reason) {
      case App.WebSocketConnectReason.ACTIVATE_ACCOUNT:
        SessionPersistence.accountActivationFailed();
        break;
      case App.WebSocketConnectReason.PASSWORD_RESET:
        SessionPersistence.resetPasswordFailed();
        break;
      default:
        SessionPersistence.loginFailed();
    }
    disconnect();
  };

  webClient.protobuf.sendSessionCommand(Data.Command_RequestPasswordSalt_ext, create(Data.Command_RequestPasswordSaltSchema, {
    ...CLIENT_CONFIG,
    userName,
  }), {
    responseExt: Data.Response_PasswordSalt_ext,
    onSuccess: (resp) => {
      const passwordSalt = resp?.passwordSalt;

      switch (options.reason) {
        case App.WebSocketConnectReason.ACTIVATE_ACCOUNT:
          activate(options, password, passwordSalt);
          break;
        case App.WebSocketConnectReason.PASSWORD_RESET:
          forgotPasswordReset(options, newPassword, passwordSalt);
          break;
        default:
          login(options, password, passwordSalt);
      }
    },
    onResponseCode: {
      [Data.Response_ResponseCode.RespRegistrationRequired]: () => {
        updateStatus(App.StatusEnum.DISCONNECTED, 'Login failed: registration required');
        onFailure();
      },
    },
    onError: () => {
      updateStatus(App.StatusEnum.DISCONNECTED, 'Login failed: Unknown Reason');
      onFailure();
    },
  });
}
