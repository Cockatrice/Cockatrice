import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Command_ForceActivateUser_ext, Command_ForceActivateUserSchema } from '@app/generated';

export function forceActivateUser(usernameToActivate: string, moderatorName: string): void {
  const cmd = create(Command_ForceActivateUserSchema, { usernameToActivate, moderatorName });
  WebClient.instance.protobuf.sendModeratorCommand(Command_ForceActivateUser_ext, cmd, {
    onSuccess: () => {
      WebClient.instance.response.moderator.forceActivateUser(usernameToActivate, moderatorName);
    },
  });
}
