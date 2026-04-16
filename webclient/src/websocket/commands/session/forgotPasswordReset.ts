import { App, Enriched, Data } from '@app/types';

import { create } from '@bufbuild/protobuf';
import type { MessageInitShape } from '@bufbuild/protobuf';
import { CLIENT_CONFIG } from '../../config';
import { WebClient } from '../../WebClient';

import { hashPassword } from '../../utils';

import { disconnect, updateStatus } from '.';

export function forgotPasswordReset(
  options: Omit<Enriched.PasswordResetConnectOptions, 'newPassword'>,
  newPassword?: string,
  passwordSalt?: string
): void {
  const { userName, token } = options;

  const params: MessageInitShape<typeof Data.Command_ForgotPasswordResetSchema> = {
    ...CLIENT_CONFIG,
    userName,
    token,
    ...(passwordSalt
      ? { hashedNewPassword: hashPassword(passwordSalt, newPassword) }
      : { newPassword }),
  };

  WebClient.instance.protobuf.sendSessionCommand(
    Data.Command_ForgotPasswordReset_ext,
    create(Data.Command_ForgotPasswordResetSchema, params),
    {
      onSuccess: () => {
        updateStatus(App.StatusEnum.DISCONNECTED, null);
        WebClient.instance.response.session.resetPasswordSuccess();
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
