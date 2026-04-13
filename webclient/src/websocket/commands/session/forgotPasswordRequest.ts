import { ForgotPasswordParams } from 'store';
import { StatusEnum, WebSocketConnectOptions } from 'types';

import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { BackendService } from '../../services/BackendService';
import {
  Command_ForgotPasswordRequest_ext, Command_ForgotPasswordRequestSchema,
} from 'generated/proto/session_commands_pb';
import { SessionPersistence } from '../../persistence';
import { Response_ForgotPasswordRequest_ext } from 'generated/proto/response_forgotpasswordrequest_pb';

import { disconnect, updateStatus } from './';

export function forgotPasswordRequest(options: WebSocketConnectOptions): void {
  const { userName } = options as unknown as ForgotPasswordParams;

  BackendService.sendSessionCommand(Command_ForgotPasswordRequest_ext, create(Command_ForgotPasswordRequestSchema, {
    ...webClient.clientConfig,
    userName,
  }), {
    responseExt: Response_ForgotPasswordRequest_ext,
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
