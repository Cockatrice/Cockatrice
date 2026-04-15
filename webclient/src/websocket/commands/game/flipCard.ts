import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { Data } from '@app/types';

export function flipCard(gameId: number, params: Data.FlipCardParams): void {
  webClient.protobuf.sendGameCommand(gameId, Data.Command_FlipCard_ext, create(Data.Command_FlipCardSchema, params));
}
