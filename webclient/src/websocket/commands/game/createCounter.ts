import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { Data } from '@app/types';

export function createCounter(gameId: number, params: Data.CreateCounterParams): void {
  webClient.protobuf.sendGameCommand(gameId, Data.Command_CreateCounter_ext, create(Data.Command_CreateCounterSchema, params));
}
