import { ForgotPasswordChallengeParams } from 'store';
import { StatusEnum, WebSocketConnectOptions } from 'types';

import webClient from '../../WebClient';
import { BackendService } from '../../services/BackendService';
import { SessionPersistence } from '../../persistence';
import { disconnect, updateStatus } from './';

export function forgotPasswordChallenge(options: WebSocketConnectOptions): void {
  const { userName, email } = options as unknown as ForgotPasswordChallengeParams;

  BackendService.sendSessionCommand('Command_ForgotPasswordChallenge', {
    ...webClient.clientConfig,
    userName,
    email,
  }, {
    onSuccess: () => {
      updateStatus(StatusEnum.DISCONNECTED, null);
      SessionPersistence.resetPassword();
      disconnect();
    },
    onError: () => {
      updateStatus(StatusEnum.DISCONNECTED, null);
      SessionPersistence.resetPasswordFailed();
      disconnect();
    },
  });
}
