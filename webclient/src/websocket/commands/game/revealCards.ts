import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_RevealCardsSchema, Command_RevealCards_ext } from 'generated/proto/command_reveal_cards_pb';
import { RevealCardsParams } from 'types';

export function revealCards(gameId: number, params: RevealCardsParams): void {
  webClient.protobuf.sendGameCommand(gameId, Command_RevealCards_ext, create(Command_RevealCardsSchema, params));
}
