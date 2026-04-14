import { ForgotPasswordChallengeParams } from 'store';
import { StatusEnum, WebSocketConnectOptions } from 'types';

import { create } from '@bufbuild/protobuf';
import { CLIENT_CONFIG } from '../../config';
import webClient from '../../WebClient';
import {
  Command_ForgotPasswordChallenge_ext, Command_ForgotPasswordChallengeSchema,
} from 'generated/proto/session_commands_pb';
import { SessionPersistence } from '../../persistence';
import { disconnect, updateStatus } from './';

export function forgotPasswordChallenge(options: WebSocketConnectOptions): void {
  const { userName, email } = options as unknown as ForgotPasswordChallengeParams;

  webClient.protobuf.sendSessionCommand(Command_ForgotPasswordChallenge_ext, create(Command_ForgotPasswordChallengeSchema, {
    ...CLIENT_CONFIG,
    userName,
    email,
  }), {
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
