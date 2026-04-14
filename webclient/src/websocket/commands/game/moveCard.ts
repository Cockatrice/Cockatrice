import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_MoveCardSchema, Command_MoveCard_ext } from 'generated/proto/command_move_card_pb';
import { MoveCardParams } from 'types';

export function moveCard(gameId: number, params: MoveCardParams): void {
  webClient.protobuf.sendGameCommand(gameId, Command_MoveCard_ext, create(Command_MoveCardSchema, params));
}
