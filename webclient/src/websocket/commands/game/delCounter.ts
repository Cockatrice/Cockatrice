import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { Data } from '@app/types';

export function delCounter(gameId: number, params: Data.DelCounterParams): void {
  webClient.protobuf.sendGameCommand(gameId, Data.Command_DelCounter_ext, create(Data.Command_DelCounterSchema, params));
}
