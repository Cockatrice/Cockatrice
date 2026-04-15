import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { Data } from '@app/types';

export function moveCard(gameId: number, params: Data.MoveCardParams): void {
  webClient.protobuf.sendGameCommand(gameId, Data.Command_MoveCard_ext, create(Data.Command_MoveCardSchema, params));
}
