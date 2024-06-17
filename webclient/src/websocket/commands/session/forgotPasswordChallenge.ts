import { ForgotPasswordChallengeParams } from 'store';
import { StatusEnum, WebSocketConnectOptions } from 'types';

import webClient from '../../WebClient';
import { SessionPersistence } from '../../persistence';
import { disconnect, updateStatus } from './';

export function forgotPasswordChallenge(options: WebSocketConnectOptions): void {
  const { userName, email } = options as unknown as ForgotPasswordChallengeParams;

  const forgotPasswordChallengeConfig = {
    ...webClient.clientConfig,
    userName,
    email,
  };

  const command = webClient.protobuf.controller.Command_ForgotPasswordChallenge.create(forgotPasswordChallengeConfig);
  const sc = webClient.protobuf.controller.SessionCommand.create({ '.Command_ForgotPasswordChallenge.ext': command });

  webClient.protobuf.sendSessionCommand(sc, raw => {
    if (raw.responseCode === webClient.protobuf.controller.Response.ResponseCode.RespOk) {
      updateStatus(StatusEnum.DISCONNECTED, null);
      SessionPersistence.resetPassword();
    } else {
      updateStatus(StatusEnum.DISCONNECTED, null);
      SessionPersistence.resetPasswordFailed();
    }

    disconnect();
  });
}
