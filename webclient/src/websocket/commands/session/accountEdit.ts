import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Command_AccountEdit_ext, Command_AccountEditSchema } from '@app/generated';

export function accountEdit(passwordCheck: string, realName?: string, email?: string, country?: string): void {
  const cmd = create(Command_AccountEditSchema, { passwordCheck, realName, email, country });
  WebClient.instance.protobuf.sendSessionCommand(Command_AccountEdit_ext, cmd, {
    onSuccess: () => {
      WebClient.instance.response.session.accountEditChanged(realName, email, country);
    },
  });
}
