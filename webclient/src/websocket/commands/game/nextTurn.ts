import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Data } from '@app/types';

export function nextTurn(gameId: number): void {
  webClient.protobuf.sendGameCommand(gameId, Data.Command_NextTurn_ext, create(Data.Command_NextTurnSchema));
}
