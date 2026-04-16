import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function replayModifyMatch(gameId: number, doNotHide: boolean): void {
  WebClient.instance.protobuf.sendSessionCommand(
    Data.Command_ReplayModifyMatch_ext,
    create(Data.Command_ReplayModifyMatchSchema, { gameId, doNotHide }),
    {
      onSuccess: () => {
        WebClient.instance.response.session.replayModifyMatch(gameId, doNotHide);
      },
    }
  );
}
