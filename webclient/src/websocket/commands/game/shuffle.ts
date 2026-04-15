import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { Data } from '@app/types';

export function shuffle(gameId: number, params: Data.ShuffleParams): void {
  webClient.protobuf.sendGameCommand(gameId, Data.Command_Shuffle_ext, create(Data.Command_ShuffleSchema, params));
}
