import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Command_DeckDel_ext, Command_DeckDelSchema } from '@app/generated';

export function deckDel(deckId: number): void {
  WebClient.instance.protobuf.sendSessionCommand(Command_DeckDel_ext, create(Command_DeckDelSchema, { deckId }), {
    onSuccess: () => {
      WebClient.instance.response.session.deleteServerDeck(deckId);
    },
  });
}
