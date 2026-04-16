import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function delCounter(gameId: number, params: Data.DelCounterParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Data.Command_DelCounter_ext, create(Data.Command_DelCounterSchema, params));
}
