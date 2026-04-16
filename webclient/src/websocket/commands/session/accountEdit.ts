import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function accountEdit(passwordCheck: string, realName?: string, email?: string, country?: string): void {
  const cmd = create(Data.Command_AccountEditSchema, { passwordCheck, realName, email, country });
  WebClient.instance.protobuf.sendSessionCommand(Data.Command_AccountEdit_ext, cmd, {
    onSuccess: () => {
      WebClient.instance.response.session.accountEditChanged(realName, email, country);
    },
  });
}
