import { ForgotPasswordResetParams } from 'store';
import { StatusEnum, WebSocketConnectOptions } from 'types';

import { create } from '@bufbuild/protobuf';
import type { MessageInitShape } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { BackendService } from '../../services/BackendService';
import {
  Command_ForgotPasswordReset_ext, Command_ForgotPasswordResetSchema,
} from 'generated/proto/session_commands_pb';
import { SessionPersistence } from '../../persistence';
import { hashPassword } from '../../utils';

import { disconnect, updateStatus } from '.';

export function forgotPasswordReset(options: WebSocketConnectOptions, newPassword?: string, passwordSalt?: string): void {
  const { userName, token } = options as unknown as ForgotPasswordResetParams;

  const params: MessageInitShape<typeof Command_ForgotPasswordResetSchema> = {
    ...webClient.clientConfig,
    userName,
    token,
    ...(passwordSalt
      ? { hashedNewPassword: hashPassword(passwordSalt, newPassword) }
      : { newPassword }),
  };

  BackendService.sendSessionCommand(Command_ForgotPasswordReset_ext, create(Command_ForgotPasswordResetSchema, params), {
    onSuccess: () => {
      updateStatus(StatusEnum.DISCONNECTED, null);
      SessionPersistence.resetPasswordSuccess();
      disconnect();
    },
    onError: () => {
      updateStatus(StatusEnum.DISCONNECTED, null);
      SessionPersistence.resetPasswordFailed();
      disconnect();
    },
  });
}
