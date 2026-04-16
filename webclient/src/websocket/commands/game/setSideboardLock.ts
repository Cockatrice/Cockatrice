import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function setSideboardLock(gameId: number, params: Data.SetSideboardLockParams): void {
  WebClient.instance.protobuf.sendGameCommand(
    gameId,
    Data.Command_SetSideboardLock_ext,
    create(Data.Command_SetSideboardLockSchema, params)
  );
}
