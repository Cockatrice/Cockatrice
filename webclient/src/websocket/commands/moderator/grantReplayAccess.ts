import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { ModeratorPersistence } from '../../persistence';
import { Data } from '@app/types';

export function grantReplayAccess(replayId: number, moderatorName: string): void {
  webClient.protobuf.sendModeratorCommand(
    Data.Command_GrantReplayAccess_ext,
    create(Data.Command_GrantReplayAccessSchema, { replayId, moderatorName }),
    {
      onSuccess: () => {
        ModeratorPersistence.grantReplayAccess(replayId, moderatorName);
      },
    },
  );
}
