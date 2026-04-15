import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { SessionPersistence } from '../../persistence';
import { Data } from '@app/types';

export function replayModifyMatch(gameId: number, doNotHide: boolean): void {
  webClient.protobuf.sendSessionCommand(
    Data.Command_ReplayModifyMatch_ext,
    create(Data.Command_ReplayModifyMatchSchema, { gameId, doNotHide }),
    {
      onSuccess: () => {
        SessionPersistence.replayModifyMatch(gameId, doNotHide);
      },
    }
  );
}
