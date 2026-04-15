import { App, Enriched, Data } from '@app/types';

import { create } from '@bufbuild/protobuf';
import { CLIENT_CONFIG } from '../../config';
import webClient from '../../WebClient';

import { SessionPersistence } from '../../persistence';

import { disconnect, updateStatus } from './';

export function forgotPasswordRequest(options: Enriched.PasswordResetRequestConnectOptions): void {
  const { userName } = options;

  webClient.protobuf.sendSessionCommand(Data.Command_ForgotPasswordRequest_ext, create(Data.Command_ForgotPasswordRequestSchema, {
    ...CLIENT_CONFIG,
    userName,
  }), {
    responseExt: Data.Response_ForgotPasswordRequest_ext,
    onSuccess: (resp) => {
      if (resp?.challengeEmail) {
        updateStatus(App.StatusEnum.DISCONNECTED, null);
        SessionPersistence.resetPasswordChallenge();
      } else {
        updateStatus(App.StatusEnum.DISCONNECTED, null);
        SessionPersistence.resetPassword();
      }
      disconnect();
    },
    onError: () => {
      updateStatus(App.StatusEnum.DISCONNECTED, null);
      SessionPersistence.resetPasswordFailed();
      disconnect();
    },
  });
}
