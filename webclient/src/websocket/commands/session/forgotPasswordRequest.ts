import { create } from '@bufbuild/protobuf';
import {
  Command_ForgotPasswordRequest_ext,
  Command_ForgotPasswordRequestSchema,
  Response_ForgotPasswordRequest_ext,
  type ForgotPasswordRequestParams,
} from '@app/generated';

import { StatusEnum } from '../../StatusEnum';
import { CLIENT_CONFIG } from '../../config';
import { WebClient } from '../../WebClient';
import type { ConnectTarget } from '../../WebClientConfig';
import { disconnect, updateStatus } from './';

export function forgotPasswordRequest(options: ConnectTarget & ForgotPasswordRequestParams): void {
  const { userName } = options;

  WebClient.instance.protobuf.sendSessionCommand(Command_ForgotPasswordRequest_ext, create(Command_ForgotPasswordRequestSchema, {
    ...CLIENT_CONFIG,
    userName,
  }), {
    responseExt: Response_ForgotPasswordRequest_ext,
    onSuccess: (resp) => {
      if (resp?.challengeEmail) {
        updateStatus(StatusEnum.DISCONNECTED, null);
        WebClient.instance.response.session.resetPasswordChallenge();
      } else {
        updateStatus(StatusEnum.DISCONNECTED, null);
        WebClient.instance.response.session.resetPassword();
      }
      disconnect();
    },
    onError: () => {
      updateStatus(StatusEnum.DISCONNECTED, null);
      WebClient.instance.response.session.resetPasswordFailed();
      disconnect();
    },
  });
}
