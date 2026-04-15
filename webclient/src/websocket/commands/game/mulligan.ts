import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { Data } from '@app/types';

export function mulligan(gameId: number, params: Data.MulliganParams): void {
  webClient.protobuf.sendGameCommand(gameId, Data.Command_Mulligan_ext, create(Data.Command_MulliganSchema, params));
}
