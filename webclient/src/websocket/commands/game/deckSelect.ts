import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_DeckSelectSchema, Command_DeckSelect_ext } from 'generated/proto/command_deck_select_pb';
import { DeckSelectParams } from 'types';

export function deckSelect(gameId: number, params: DeckSelectParams): void {
  webClient.protobuf.sendGameCommand(gameId, Command_DeckSelect_ext, create(Command_DeckSelectSchema, params));
}
