import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Command_AccountPassword_ext, Command_AccountPasswordSchema } from '@app/generated';

export function accountPassword(oldPassword: string, newPassword: string, hashedNewPassword: string): void {
  const cmd = create(Command_AccountPasswordSchema, { oldPassword, newPassword, hashedNewPassword });
  WebClient.instance.protobuf.sendSessionCommand(Command_AccountPassword_ext, cmd, {
    onSuccess: () => {
      WebClient.instance.response.session.accountPasswordChange();
    },
  });
}
