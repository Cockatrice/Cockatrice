import { create } from '@bufbuild/protobuf';
import type { MessageInitShape } from '@bufbuild/protobuf';
import {
  Command_ForgotPasswordReset_ext,
  Command_ForgotPasswordResetSchema,
  type ForgotPasswordResetParams,
} from '@app/generated';

import { StatusEnum } from '../../types/StatusEnum';
import { CLIENT_CONFIG } from '../../config';
import { WebClient } from '../../WebClient';
import type { ConnectTarget } from '../../types/WebClientConfig';
import { hashPassword } from '../../utils';
import { disconnect, updateStatus } from '.';

export function forgotPasswordReset(
  options: ConnectTarget & ForgotPasswordResetParams,
  newPassword?: string,
  passwordSalt?: string
): void {
  const { userName, token } = options;

  const params: MessageInitShape<typeof Command_ForgotPasswordResetSchema> = {
    ...CLIENT_CONFIG,
    userName,
    token,
    ...(passwordSalt
      ? { hashedNewPassword: hashPassword(passwordSalt, newPassword) }
      : { newPassword }),
  };

  WebClient.instance.protobuf.sendSessionCommand(
    Command_ForgotPasswordReset_ext,
    create(Command_ForgotPasswordResetSchema, params),
    {
      onSuccess: () => {
        updateStatus(StatusEnum.DISCONNECTED, null);
        WebClient.instance.response.session.resetPasswordSuccess();
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
