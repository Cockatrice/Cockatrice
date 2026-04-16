import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function deckDownload(deckId: number): void {
  WebClient.instance.protobuf.sendSessionCommand(
    Data.Command_DeckDownload_ext,
    create(Data.Command_DeckDownloadSchema, { deckId }),
    {
      responseExt: Data.Response_DeckDownload_ext,
      onSuccess: (response) => {
        WebClient.instance.response.session.downloadServerDeck(deckId, response);
      },
    }
  );
}
