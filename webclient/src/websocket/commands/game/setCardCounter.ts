import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { Data } from '@app/types';

export function setCardCounter(gameId: number, params: Data.SetCardCounterParams): void {
  webClient.protobuf.sendGameCommand(gameId, Data.Command_SetCardCounter_ext, create(Data.Command_SetCardCounterSchema, params));
}
