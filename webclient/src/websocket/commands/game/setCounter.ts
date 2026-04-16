import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function setCounter(gameId: number, params: Data.SetCounterParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Data.Command_SetCounter_ext, create(Data.Command_SetCounterSchema, params));
}
