import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function incCounter(gameId: number, params: Data.IncCounterParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Data.Command_IncCounter_ext, create(Data.Command_IncCounterSchema, params));
}
