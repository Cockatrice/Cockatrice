import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { Data } from '@app/types';

export function deckSelect(gameId: number, params: Data.DeckSelectParams): void {
  webClient.protobuf.sendGameCommand(gameId, Data.Command_DeckSelect_ext, create(Data.Command_DeckSelectSchema, params));
}
