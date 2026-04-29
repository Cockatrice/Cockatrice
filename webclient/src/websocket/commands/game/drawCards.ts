import { create } from '@bufbuild/protobuf';
import { Command_DrawCards_ext, Command_DrawCardsSchema, type DrawCardsParams } from '@app/generated';
import { WebClient } from '../../WebClient';

export function drawCards(gameId: number, params: DrawCardsParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Command_DrawCards_ext, create(Command_DrawCardsSchema, params));
}
