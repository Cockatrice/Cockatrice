import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_LeaveGameSchema, Command_LeaveGame_ext } from 'generated/proto/command_leave_game_pb';

export function leaveGame(gameId: number): void {
  BackendService.sendGameCommand(gameId, Command_LeaveGame_ext, create(Command_LeaveGameSchema));
}
