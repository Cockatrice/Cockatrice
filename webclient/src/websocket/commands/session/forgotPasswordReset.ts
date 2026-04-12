import { ForgotPasswordResetParams } from 'store';
import { StatusEnum, WebSocketConnectOptions } from 'types';

import webClient from '../../WebClient';
import { BackendService } from '../../services/BackendService';
import { SessionPersistence } from '../../persistence';
import { hashPassword } from '../../utils';

import { disconnect, updateStatus } from '.';

export function forgotPasswordReset(options: WebSocketConnectOptions, passwordSalt?: string): void {
  const { userName, token, newPassword } = options as unknown as ForgotPasswordResetParams;

  const params: any = {
    ...webClient.clientConfig,
    userName,
    token,
  };

  if (passwordSalt) {
    params.hashedNewPassword = hashPassword(passwordSalt, newPassword);
  } else {
    params.newPassword = newPassword;
  }

  BackendService.sendSessionCommand('Command_ForgotPasswordReset', params, {
    onSuccess: () => {
      updateStatus(StatusEnum.DISCONNECTED, null);
      SessionPersistence.resetPasswordSuccess();
      disconnect();
    },
    onError: () => {
      updateStatus(StatusEnum.DISCONNECTED, null);
      SessionPersistence.resetPasswordFailed();
      disconnect();
    },
  });
}
