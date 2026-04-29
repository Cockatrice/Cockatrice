import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Command_ReplayModifyMatch_ext, Command_ReplayModifyMatchSchema } from '@app/generated';

export function replayModifyMatch(gameId: number, doNotHide: boolean): void {
  WebClient.instance.protobuf.sendSessionCommand(
    Command_ReplayModifyMatch_ext,
    create(Command_ReplayModifyMatchSchema, { gameId, doNotHide }),
    {
      onSuccess: () => {
        WebClient.instance.response.session.replayModifyMatch(gameId, doNotHide);
      },
    }
  );
}
