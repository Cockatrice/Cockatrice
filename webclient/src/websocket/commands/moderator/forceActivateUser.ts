import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { ModeratorPersistence } from '../../persistence';
import { Data } from '@app/types';

export function forceActivateUser(usernameToActivate: string, moderatorName: string): void {
  const cmd = create(Data.Command_ForceActivateUserSchema, { usernameToActivate, moderatorName });
  webClient.protobuf.sendModeratorCommand(Data.Command_ForceActivateUser_ext, cmd, {
    onSuccess: () => {
      ModeratorPersistence.forceActivateUser(usernameToActivate, moderatorName);
    },
  });
}
