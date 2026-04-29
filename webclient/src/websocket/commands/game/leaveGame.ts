import { create } from '@bufbuild/protobuf';
import { Command_LeaveGame_ext, Command_LeaveGameSchema } from '@app/generated';
import { WebClient } from '../../WebClient';

export function leaveGame(gameId: number): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Command_LeaveGame_ext, create(Command_LeaveGameSchema));
}
