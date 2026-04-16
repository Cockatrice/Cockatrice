import { create } from '@bufbuild/protobuf';
import { Command_MoveCard_ext, Command_MoveCardSchema, type MoveCardParams } from '@app/generated';
import { WebClient } from '../../WebClient';

export function moveCard(gameId: number, params: MoveCardParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Command_MoveCard_ext, create(Command_MoveCardSchema, params));
}
