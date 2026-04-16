import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function createCounter(gameId: number, params: Data.CreateCounterParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Data.Command_CreateCounter_ext, create(Data.Command_CreateCounterSchema, params));
}
