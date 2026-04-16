import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function revealCards(gameId: number, params: Data.RevealCardsParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Data.Command_RevealCards_ext, create(Data.Command_RevealCardsSchema, params));
}
