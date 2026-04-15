import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { Data } from '@app/types';

export function incCounter(gameId: number, params: Data.IncCounterParams): void {
  webClient.protobuf.sendGameCommand(gameId, Data.Command_IncCounter_ext, create(Data.Command_IncCounterSchema, params));
}
