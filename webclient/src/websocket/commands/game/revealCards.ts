import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { Data } from '@app/types';

export function revealCards(gameId: number, params: Data.RevealCardsParams): void {
  webClient.protobuf.sendGameCommand(gameId, Data.Command_RevealCards_ext, create(Data.Command_RevealCardsSchema, params));
}
