import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { SessionPersistence } from '../../persistence';
import { Data } from '@app/types';

export function deckUpload(path: string, deckId: number, deckList: string): void {
  webClient.protobuf.sendSessionCommand(Data.Command_DeckUpload_ext, create(Data.Command_DeckUploadSchema, { path, deckId, deckList }), {
    responseExt: Data.Response_DeckUpload_ext,
    onSuccess: (response) => {
      if (response.newFile) {
        SessionPersistence.uploadServerDeck(path, response.newFile);
      }
    },
  });
}
