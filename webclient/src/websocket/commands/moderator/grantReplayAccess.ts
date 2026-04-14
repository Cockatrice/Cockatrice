import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import {
  Command_GrantReplayAccess_ext, Command_GrantReplayAccessSchema,
} from 'generated/proto/moderator_commands_pb';
import { ModeratorPersistence } from '../../persistence';

export function grantReplayAccess(replayId: number, moderatorName: string): void {
  webClient.protobuf.sendModeratorCommand(
    Command_GrantReplayAccess_ext,
    create(Command_GrantReplayAccessSchema, { replayId, moderatorName }),
    {
      onSuccess: () => {
        ModeratorPersistence.grantReplayAccess(replayId, moderatorName);
      },
    },
  );
}
