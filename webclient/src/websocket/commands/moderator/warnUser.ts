import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { ModeratorPersistence } from '../../persistence';
import { Data } from '@app/types';

export function warnUser(userName: string, reason: string, clientid?: string, removeMessages?: number): void {
  const cmd = create(Data.Command_WarnUserSchema, { userName, reason, clientid, removeMessages });
  webClient.protobuf.sendModeratorCommand(Data.Command_WarnUser_ext, cmd, {
    onSuccess: () => {
      ModeratorPersistence.warnUser(userName);
    },
  });
}
