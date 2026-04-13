import { RequestPasswordSaltParams } from 'store';
import { StatusEnum, WebSocketConnectOptions, WebSocketConnectReason } from 'types';

import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { BackendService } from '../../services/BackendService';
import {
  Command_RequestPasswordSalt_ext, Command_RequestPasswordSaltSchema,
} from 'generated/proto/session_commands_pb';
import { SessionPersistence } from '../../persistence';
import { Response_PasswordSalt_ext } from 'generated/proto/response_password_salt_pb';
import { Response_ResponseCode } from 'generated/proto/response_pb';

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

  BackendService.sendSessionCommand(Command_RequestPasswordSalt_ext, create(Command_RequestPasswordSaltSchema, {
    ...webClient.clientConfig,
    userName,
  }), {
    responseExt: Response_PasswordSalt_ext,
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
      [Response_ResponseCode.RespRegistrationRequired]: () => {
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
