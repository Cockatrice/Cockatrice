import { App, Enriched, Data } from '@app/types';

import { create } from '@bufbuild/protobuf';
import { CLIENT_CONFIG } from '../../config';
import { WebClient } from '../../WebClient';

import { disconnect, updateStatus } from './';

export function forgotPasswordRequest(options: Enriched.PasswordResetRequestConnectOptions): void {
  const { userName } = options;

  WebClient.instance.protobuf.sendSessionCommand(Data.Command_ForgotPasswordRequest_ext, create(Data.Command_ForgotPasswordRequestSchema, {
    ...CLIENT_CONFIG,
    userName,
  }), {
    responseExt: Data.Response_ForgotPasswordRequest_ext,
    onSuccess: (resp) => {
      if (resp?.challengeEmail) {
        updateStatus(App.StatusEnum.DISCONNECTED, null);
        WebClient.instance.response.session.resetPasswordChallenge();
      } else {
        updateStatus(App.StatusEnum.DISCONNECTED, null);
        WebClient.instance.response.session.resetPassword();
      }
      disconnect();
    },
    onError: () => {
      updateStatus(App.StatusEnum.DISCONNECTED, null);
      WebClient.instance.response.session.resetPasswordFailed();
      disconnect();
    },
  });
}
