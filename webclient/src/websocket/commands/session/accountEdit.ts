import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { SessionPersistence } from '../../persistence';
import { Data } from '@app/types';

export function accountEdit(passwordCheck: string, realName?: string, email?: string, country?: string): void {
  const cmd = create(Data.Command_AccountEditSchema, { passwordCheck, realName, email, country });
  webClient.protobuf.sendSessionCommand(Data.Command_AccountEdit_ext, cmd, {
    onSuccess: () => {
      SessionPersistence.accountEditChanged(realName, email, country);
    },
  });
}
