import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { SessionPersistence } from '../../persistence';
import { Data } from '@app/types';

export function deckList(): void {
  webClient.protobuf.sendSessionCommand(Data.Command_DeckList_ext, create(Data.Command_DeckListSchema), {
    responseExt: Data.Response_DeckList_ext,
    onSuccess: (response) => {
      if (response.root) {
        SessionPersistence.updateServerDecks(response);
      }
    },
  });
}
