import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { SessionPersistence } from '../../persistence';
import { Data } from '@app/types';

export function replayDeleteMatch(gameId: number): void {
  webClient.protobuf.sendSessionCommand(Data.Command_ReplayDeleteMatch_ext, create(Data.Command_ReplayDeleteMatchSchema, { gameId }), {
    onSuccess: () => {
      SessionPersistence.replayDeleteMatch(gameId);
    },
  });
}
