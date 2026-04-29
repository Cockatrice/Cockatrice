import { create } from '@bufbuild/protobuf';
import { Command_RevealCards_ext, Command_RevealCardsSchema, type RevealCardsParams } from '@app/generated';
import { WebClient } from '../../WebClient';

export function revealCards(gameId: number, params: RevealCardsParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Command_RevealCards_ext, create(Command_RevealCardsSchema, params));
}
