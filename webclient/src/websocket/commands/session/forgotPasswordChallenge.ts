import { create } from '@bufbuild/protobuf';
import {
  Command_ForgotPasswordChallenge_ext,
  Command_ForgotPasswordChallengeSchema,
  type ForgotPasswordChallengeParams,
} from '@app/generated';

import { StatusEnum } from '../../StatusEnum';
import { CLIENT_CONFIG } from '../../config';
import { WebClient } from '../../WebClient';
import type { ConnectTarget } from '../../WebClientConfig';
import { disconnect, updateStatus } from './';

export function forgotPasswordChallenge(options: ConnectTarget & ForgotPasswordChallengeParams): void {
  const { userName, email } = options;

  WebClient.instance.protobuf.sendSessionCommand(
    Command_ForgotPasswordChallenge_ext,
    create(Command_ForgotPasswordChallengeSchema, {
      ...CLIENT_CONFIG,
      userName,
      email,
    }),
    {
      onSuccess: () => {
        updateStatus(StatusEnum.DISCONNECTED, null);
        WebClient.instance.response.session.resetPassword();
        disconnect();
      },
      onError: () => {
        updateStatus(StatusEnum.DISCONNECTED, null);
        WebClient.instance.response.session.resetPasswordFailed();
        disconnect();
      },
    }
  );
}
