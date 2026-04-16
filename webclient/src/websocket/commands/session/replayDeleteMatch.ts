import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function replayDeleteMatch(gameId: number): void {
  WebClient.instance.protobuf.sendSessionCommand(
    Data.Command_ReplayDeleteMatch_ext,
    create(Data.Command_ReplayDeleteMatchSchema, { gameId }),
    {
      onSuccess: () => {
        WebClient.instance.response.session.replayDeleteMatch(gameId);
      },
    }
  );
}
