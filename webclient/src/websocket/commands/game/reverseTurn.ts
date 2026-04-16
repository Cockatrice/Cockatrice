import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';
import { Data } from '@app/types';

export function reverseTurn(gameId: number): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Data.Command_ReverseTurn_ext, create(Data.Command_ReverseTurnSchema));
}
