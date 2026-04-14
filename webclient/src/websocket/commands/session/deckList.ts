import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_DeckListSchema, Command_DeckList_ext } from 'generated/proto/command_deck_list_pb';
import { SessionPersistence } from '../../persistence';
import { Response_DeckList_ext } from 'generated/proto/response_deck_list_pb';

export function deckList(): void {
  BackendService.sendSessionCommand(Command_DeckList_ext, create(Command_DeckListSchema), {
    responseExt: Response_DeckList_ext,
    onSuccess: (response) => {
      if (response.root) {
        SessionPersistence.updateServerDecks(response);
      }
    },
  });
}
