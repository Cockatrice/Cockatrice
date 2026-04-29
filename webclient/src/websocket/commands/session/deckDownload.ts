import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Command_DeckDownload_ext, Command_DeckDownloadSchema, Response_DeckDownload_ext } from '@app/generated';

export function deckDownload(deckId: number): void {
  WebClient.instance.protobuf.sendSessionCommand(
    Command_DeckDownload_ext,
    create(Command_DeckDownloadSchema, { deckId }),
    {
      responseExt: Response_DeckDownload_ext,
      onSuccess: (response) => {
        WebClient.instance.response.session.downloadServerDeck(deckId, response);
      },
    }
  );
}
