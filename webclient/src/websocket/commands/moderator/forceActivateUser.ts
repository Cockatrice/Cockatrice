import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import {
  Command_ForceActivateUser_ext, Command_ForceActivateUserSchema,
} from 'generated/proto/moderator_commands_pb';
import { ModeratorPersistence } from '../../persistence';

export function forceActivateUser(usernameToActivate: string, moderatorName: string): void {
  const cmd = create(Command_ForceActivateUserSchema, { usernameToActivate, moderatorName });
  webClient.protobuf.sendModeratorCommand(Command_ForceActivateUser_ext, cmd, {
    onSuccess: () => {
      ModeratorPersistence.forceActivateUser(usernameToActivate, moderatorName);
    },
  });
}
