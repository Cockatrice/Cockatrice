import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function deckList(): void {
  WebClient.instance.protobuf.sendSessionCommand(Data.Command_DeckList_ext, create(Data.Command_DeckListSchema), {
    responseExt: Data.Response_DeckList_ext,
    onSuccess: (response) => {
      if (response.root) {
        WebClient.instance.response.session.updateServerDecks(response);
      }
    },
  });
}
