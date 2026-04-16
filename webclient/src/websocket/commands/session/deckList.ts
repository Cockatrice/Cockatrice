import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Command_DeckList_ext, Command_DeckListSchema, Response_DeckList_ext } from '@app/generated';

export function deckList(): void {
  WebClient.instance.protobuf.sendSessionCommand(Command_DeckList_ext, create(Command_DeckListSchema), {
    responseExt: Response_DeckList_ext,
    onSuccess: (response) => {
      if (response.root) {
        WebClient.instance.response.session.updateServerDecks(response);
      }
    },
  });
}
