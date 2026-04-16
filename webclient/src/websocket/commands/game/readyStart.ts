import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function readyStart(gameId: number, params: Data.ReadyStartParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Data.Command_ReadyStart_ext, create(Data.Command_ReadyStartSchema, params));
}
