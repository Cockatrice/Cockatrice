import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { Data } from '@app/types';

export function drawCards(gameId: number, params: Data.DrawCardsParams): void {
  webClient.protobuf.sendGameCommand(gameId, Data.Command_DrawCards_ext, create(Data.Command_DrawCardsSchema, params));
}
