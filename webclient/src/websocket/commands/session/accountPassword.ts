import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function accountPassword(oldPassword: string, newPassword: string, hashedNewPassword: string): void {
  const cmd = create(Data.Command_AccountPasswordSchema, { oldPassword, newPassword, hashedNewPassword });
  WebClient.instance.protobuf.sendSessionCommand(Data.Command_AccountPassword_ext, cmd, {
    onSuccess: () => {
      WebClient.instance.response.session.accountPasswordChange();
    },
  });
}
