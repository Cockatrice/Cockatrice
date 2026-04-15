import { ForgotPasswordParams } from 'store';
import { StatusEnum, WebSocketConnectOptions } from 'types';

import webClient from '../../WebClient';
import { BackendService } from '../../services/BackendService';
import { SessionPersistence } from '../../persistence';

import { disconnect, updateStatus } from './';

export function forgotPasswordRequest(options: WebSocketConnectOptions): void {
  const { userName } = options as unknown as ForgotPasswordParams;

  BackendService.sendSessionCommand('Command_ForgotPasswordRequest', {
    ...webClient.clientConfig,
    userName,
  }, {
    responseName: 'Response_ForgotPasswordRequest',
    onSuccess: (resp) => {
      if (resp?.challengeEmail) {
        updateStatus(StatusEnum.DISCONNECTED, null);
        SessionPersistence.resetPasswordChallenge();
      } else {
        updateStatus(StatusEnum.DISCONNECTED, null);
        SessionPersistence.resetPassword();
      }
      disconnect();
    },
    onError: () => {
      updateStatus(StatusEnum.DISCONNECTED, null);
      SessionPersistence.resetPasswordFailed();
      disconnect();
    },
  });
}
