import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_DrawCardsSchema, Command_DrawCards_ext } from 'generated/proto/command_draw_cards_pb';
import { DrawCardsParams } from 'types';

export function drawCards(gameId: number, params: DrawCardsParams): void {
  BackendService.sendGameCommand(gameId, Command_DrawCards_ext, create(Command_DrawCardsSchema, params));
}
