import { ForgotPasswordResetParams } from 'store';
import { StatusEnum, WebSocketConnectOptions } from 'types';

import webClient from '../../WebClient';
import { SessionPersistence } from '../../persistence';
import { hashPassword } from '../../utils';

import { disconnect, updateStatus } from '.';

export function forgotPasswordReset(options: WebSocketConnectOptions, passwordSalt?: string): void {
  const { userName, token, newPassword } = options as unknown as ForgotPasswordResetParams;

  const forgotPasswordResetConfig: any = {
    ...webClient.clientConfig,
    userName,
    token,
  };

  if (passwordSalt) {
    forgotPasswordResetConfig.hashedNewPassword = hashPassword(passwordSalt, newPassword);
  } else {
    forgotPasswordResetConfig.newPassword = newPassword;
  }

  const command = webClient.protobuf.controller.Command_ForgotPasswordReset.create(forgotPasswordResetConfig);
  const sc = webClient.protobuf.controller.SessionCommand.create({ '.Command_ForgotPasswordReset.ext': command });

  webClient.protobuf.sendSessionCommand(sc, raw => {
    if (raw.responseCode === webClient.protobuf.controller.Response.ResponseCode.RespOk) {
      updateStatus(StatusEnum.DISCONNECTED, null);
      SessionPersistence.resetPasswordSuccess();
    } else {
      updateStatus(StatusEnum.DISCONNECTED, null);
      SessionPersistence.resetPasswordFailed();
    }

    disconnect();
  });
}
