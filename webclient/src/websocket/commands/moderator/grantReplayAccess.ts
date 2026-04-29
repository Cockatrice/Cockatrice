import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Command_GrantReplayAccess_ext, Command_GrantReplayAccessSchema } from '@app/generated';

export function grantReplayAccess(replayId: number, moderatorName: string): void {
  WebClient.instance.protobuf.sendModeratorCommand(
    Command_GrantReplayAccess_ext,
    create(Command_GrantReplayAccessSchema, { replayId, moderatorName }),
    {
      onSuccess: () => {
        WebClient.instance.response.moderator.grantReplayAccess(replayId, moderatorName);
      },
    },
  );
}
