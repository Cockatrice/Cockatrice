import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Command_DeckUpload_ext, Command_DeckUploadSchema, Response_DeckUpload_ext } from '@app/generated';

export function deckUpload(path: string, deckId: number, deckList: string): void {
  WebClient.instance.protobuf.sendSessionCommand(
    Command_DeckUpload_ext,
    create(Command_DeckUploadSchema, { path, deckId, deckList }),
    {
      responseExt: Response_DeckUpload_ext,
      onSuccess: (response) => {
        if (response.newFile) {
          WebClient.instance.response.session.uploadServerDeck(path, response.newFile);
        }
      },
    }
  );
}
