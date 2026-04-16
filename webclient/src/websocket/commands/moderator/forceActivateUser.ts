import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function forceActivateUser(usernameToActivate: string, moderatorName: string): void {
  const cmd = create(Data.Command_ForceActivateUserSchema, { usernameToActivate, moderatorName });
  WebClient.instance.protobuf.sendModeratorCommand(Data.Command_ForceActivateUser_ext, cmd, {
    onSuccess: () => {
      WebClient.instance.response.moderator.forceActivateUser(usernameToActivate, moderatorName);
    },
  });
}
