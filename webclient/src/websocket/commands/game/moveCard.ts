import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_MoveCardSchema, Command_MoveCard_ext } from 'generated/proto/command_move_card_pb';
import { MoveCardParams } from 'types';

export function moveCard(gameId: number, params: MoveCardParams): void {
  BackendService.sendGameCommand(gameId, Command_MoveCard_ext, create(Command_MoveCardSchema, params));
}
