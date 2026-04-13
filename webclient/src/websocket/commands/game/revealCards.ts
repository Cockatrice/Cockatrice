import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_RevealCardsSchema, Command_RevealCards_ext } from 'generated/proto/command_reveal_cards_pb';
import { RevealCardsParams } from 'types';

export function revealCards(gameId: number, params: RevealCardsParams): void {
  BackendService.sendGameCommand(gameId, Command_RevealCards_ext, create(Command_RevealCardsSchema, params));
}
