import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function warnUser(userName: string, reason: string, clientid?: string, removeMessages?: number): void {
  const cmd = create(Data.Command_WarnUserSchema, { userName, reason, clientid, removeMessages });
  WebClient.instance.protobuf.sendModeratorCommand(Data.Command_WarnUser_ext, cmd, {
    onSuccess: () => {
      WebClient.instance.response.moderator.warnUser(userName);
    },
  });
}
