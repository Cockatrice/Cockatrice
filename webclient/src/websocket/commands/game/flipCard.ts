import { create } from '@bufbuild/protobuf';
import { Command_FlipCard_ext, Command_FlipCardSchema, type FlipCardParams } from '@app/generated';
import { WebClient } from '../../WebClient';

export function flipCard(gameId: number, params: FlipCardParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Command_FlipCard_ext, create(Command_FlipCardSchema, params));
}
