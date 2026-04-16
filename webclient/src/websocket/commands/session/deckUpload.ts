import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function deckUpload(path: string, deckId: number, deckList: string): void {
  WebClient.instance.protobuf.sendSessionCommand(
    Data.Command_DeckUpload_ext,
    create(Data.Command_DeckUploadSchema, { path, deckId, deckList }),
    {
      responseExt: Data.Response_DeckUpload_ext,
      onSuccess: (response) => {
        if (response.newFile) {
          WebClient.instance.response.session.uploadServerDeck(path, response.newFile);
        }
      },
    }
  );
}
