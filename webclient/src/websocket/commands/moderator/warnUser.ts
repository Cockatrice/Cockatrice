import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Command_WarnUser_ext, Command_WarnUserSchema } from '@app/generated';

export function warnUser(userName: string, reason: string, clientid?: string, removeMessages?: number): void {
  const cmd = create(Command_WarnUserSchema, { userName, reason, clientid, removeMessages });
  WebClient.instance.protobuf.sendModeratorCommand(Command_WarnUser_ext, cmd, {
    onSuccess: () => {
      WebClient.instance.response.moderator.warnUser(userName);
    },
  });
}
