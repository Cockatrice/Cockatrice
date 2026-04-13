import { ForgotPasswordChallengeParams } from 'store';
import { StatusEnum, WebSocketConnectOptions } from 'types';

import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { BackendService } from '../../services/BackendService';
import {
  Command_ForgotPasswordChallenge_ext, Command_ForgotPasswordChallengeSchema,
} from 'generated/proto/session_commands_pb';
import { SessionPersistence } from '../../persistence';
import { disconnect, updateStatus } from './';

export function forgotPasswordChallenge(options: WebSocketConnectOptions): void {
  const { userName, email } = options as unknown as ForgotPasswordChallengeParams;

  BackendService.sendSessionCommand(Command_ForgotPasswordChallenge_ext, create(Command_ForgotPasswordChallengeSchema, {
    ...webClient.clientConfig,
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
