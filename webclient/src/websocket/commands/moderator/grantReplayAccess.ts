import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function grantReplayAccess(replayId: number, moderatorName: string): void {
  WebClient.instance.protobuf.sendModeratorCommand(
    Data.Command_GrantReplayAccess_ext,
    create(Data.Command_GrantReplayAccessSchema, { replayId, moderatorName }),
    {
      onSuccess: () => {
        WebClient.instance.response.moderator.grantReplayAccess(replayId, moderatorName);
      },
    },
  );
}
