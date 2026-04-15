import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Data } from '@app/types';

export function leaveGame(gameId: number): void {
  webClient.protobuf.sendGameCommand(gameId, Data.Command_LeaveGame_ext, create(Data.Command_LeaveGameSchema));
}
