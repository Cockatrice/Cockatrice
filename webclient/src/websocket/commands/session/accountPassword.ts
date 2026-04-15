import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { SessionPersistence } from '../../persistence';
import { Data } from '@app/types';

export function accountPassword(oldPassword: string, newPassword: string, hashedNewPassword: string): void {
  const cmd = create(Data.Command_AccountPasswordSchema, { oldPassword, newPassword, hashedNewPassword });
  webClient.protobuf.sendSessionCommand(Data.Command_AccountPassword_ext, cmd, {
    onSuccess: () => {
      SessionPersistence.accountPasswordChange();
    },
  });
}
