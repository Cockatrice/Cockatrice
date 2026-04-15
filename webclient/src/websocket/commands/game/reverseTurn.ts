import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Data } from '@app/types';

export function reverseTurn(gameId: number): void {
  webClient.protobuf.sendGameCommand(gameId, Data.Command_ReverseTurn_ext, create(Data.Command_ReverseTurnSchema));
}
