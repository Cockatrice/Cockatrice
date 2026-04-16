import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function deckDel(deckId: number): void {
  WebClient.instance.protobuf.sendSessionCommand(Data.Command_DeckDel_ext, create(Data.Command_DeckDelSchema, { deckId }), {
    onSuccess: () => {
      WebClient.instance.response.session.deleteServerDeck(deckId);
    },
  });
}
