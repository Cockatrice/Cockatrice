import { create } from '@bufbuild/protobuf';
import { Command_DeckSelect_ext, Command_DeckSelectSchema, type DeckSelectParams } from '@app/generated';
import { WebClient } from '../../WebClient';

export function deckSelect(gameId: number, params: DeckSelectParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Command_DeckSelect_ext, create(Command_DeckSelectSchema, params));
}
