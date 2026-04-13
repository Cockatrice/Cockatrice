import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_NextTurnSchema, Command_NextTurn_ext } from 'generated/proto/command_next_turn_pb';

export function nextTurn(gameId: number): void {
  BackendService.sendGameCommand(gameId, Command_NextTurn_ext, create(Command_NextTurnSchema));
}
