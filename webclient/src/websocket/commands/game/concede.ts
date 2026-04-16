import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';
import { Data } from '@app/types';

export function concede(gameId: number): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Data.Command_Concede_ext, create(Data.Command_ConcedeSchema));
}
