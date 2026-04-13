import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_ReverseTurnSchema, Command_ReverseTurn_ext } from 'generated/proto/command_reverse_turn_pb';

export function reverseTurn(gameId: number): void {
  BackendService.sendGameCommand(gameId, Command_ReverseTurn_ext, create(Command_ReverseTurnSchema));
}
