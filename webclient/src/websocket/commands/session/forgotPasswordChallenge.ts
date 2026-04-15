import { App, Enriched, Data } from '@app/types';

import { create } from '@bufbuild/protobuf';
import { CLIENT_CONFIG } from '../../config';
import webClient from '../../WebClient';

import { SessionPersistence } from '../../persistence';
import { disconnect, updateStatus } from './';

export function forgotPasswordChallenge(options: Enriched.PasswordResetChallengeConnectOptions): void {
  const { userName, email } = options;

  webClient.protobuf.sendSessionCommand(Data.Command_ForgotPasswordChallenge_ext, create(Data.Command_ForgotPasswordChallengeSchema, {
    ...CLIENT_CONFIG,
    userName,
    email,
  }), {
    onSuccess: () => {
      updateStatus(App.StatusEnum.DISCONNECTED, null);
      SessionPersistence.resetPassword();
      disconnect();
    },
    onError: () => {
      updateStatus(App.StatusEnum.DISCONNECTED, null);
      SessionPersistence.resetPasswordFailed();
      disconnect();
    },
  });
}
