import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_DeckUploadSchema, Command_DeckUpload_ext } from 'generated/proto/command_deck_upload_pb';
import { SessionPersistence } from '../../persistence';
import { Response_DeckUpload_ext } from 'generated/proto/response_deck_upload_pb';

export function deckUpload(path: string, deckId: number, deckList: string): void {
  webClient.protobuf.sendSessionCommand(Command_DeckUpload_ext, create(Command_DeckUploadSchema, { path, deckId, deckList }), {
    responseExt: Response_DeckUpload_ext,
    onSuccess: (response) => {
      if (response.newFile) {
        SessionPersistence.uploadServerDeck(path, response.newFile);
      }
    },
  });
}
