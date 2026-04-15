import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { SessionPersistence } from '../../persistence';
import { Data } from '@app/types';

export function deckDel(deckId: number): void {
  webClient.protobuf.sendSessionCommand(Data.Command_DeckDel_ext, create(Data.Command_DeckDelSchema, { deckId }), {
    onSuccess: () => {
      SessionPersistence.deleteServerDeck(deckId);
    },
  });
}
