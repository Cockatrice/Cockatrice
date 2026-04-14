import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_LeaveGameSchema, Command_LeaveGame_ext } from 'generated/proto/command_leave_game_pb';

export function leaveGame(gameId: number): void {
  webClient.protobuf.sendGameCommand(gameId, Command_LeaveGame_ext, create(Command_LeaveGameSchema));
}
