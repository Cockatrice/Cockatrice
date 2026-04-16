import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Command_ReplayDeleteMatch_ext, Command_ReplayDeleteMatchSchema } from '@app/generated';

export function replayDeleteMatch(gameId: number): void {
  WebClient.instance.protobuf.sendSessionCommand(
    Command_ReplayDeleteMatch_ext,
    create(Command_ReplayDeleteMatchSchema, { gameId }),
    {
      onSuccess: () => {
        WebClient.instance.response.session.replayDeleteMatch(gameId);
      },
    }
  );
}
