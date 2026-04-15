import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { Data } from '@app/types';

export function incCardCounter(gameId: number, params: Data.IncCardCounterParams): void {
  webClient.protobuf.sendGameCommand(gameId, Data.Command_IncCardCounter_ext, create(Data.Command_IncCardCounterSchema, params));
}
