import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';
import { Data } from '@app/types';

export function leaveGame(gameId: number): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Data.Command_LeaveGame_ext, create(Data.Command_LeaveGameSchema));
}
