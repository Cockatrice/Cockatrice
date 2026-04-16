import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';
import { Data } from '@app/types';

export function nextTurn(gameId: number): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Data.Command_NextTurn_ext, create(Data.Command_NextTurnSchema));
}
