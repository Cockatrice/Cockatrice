import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function incCardCounter(gameId: number, params: Data.IncCardCounterParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Data.Command_IncCardCounter_ext, create(Data.Command_IncCardCounterSchema, params));
}
