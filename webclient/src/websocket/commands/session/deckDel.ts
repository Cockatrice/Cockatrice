import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_DeckDelSchema, Command_DeckDel_ext } from 'generated/proto/command_deck_del_pb';
import { SessionPersistence } from '../../persistence';

export function deckDel(deckId: number): void {
  webClient.protobuf.sendSessionCommand(Command_DeckDel_ext, create(Command_DeckDelSchema, { deckId }), {
    onSuccess: () => {
      SessionPersistence.deleteServerDeck(deckId);
    },
  });
}
