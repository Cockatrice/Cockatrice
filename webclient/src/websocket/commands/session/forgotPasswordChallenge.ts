import { App, Enriched, Data } from '@app/types';

import { create } from '@bufbuild/protobuf';
import { CLIENT_CONFIG } from '../../config';
import { WebClient } from '../../WebClient';

import { disconnect, updateStatus } from './';

export function forgotPasswordChallenge(options: Enriched.PasswordResetChallengeConnectOptions): void {
  const { userName, email } = options;

  WebClient.instance.protobuf.sendSessionCommand(
    Data.Command_ForgotPasswordChallenge_ext,
    create(Data.Command_ForgotPasswordChallengeSchema, {
      ...CLIENT_CONFIG,
      userName,
      email,
    }),
    {
      onSuccess: () => {
        updateStatus(App.StatusEnum.DISCONNECTED, null);
        WebClient.instance.response.session.resetPassword();
        disconnect();
      },
      onError: () => {
        updateStatus(App.StatusEnum.DISCONNECTED, null);
        WebClient.instance.response.session.resetPasswordFailed();
        disconnect();
      },
    }
  );
}
