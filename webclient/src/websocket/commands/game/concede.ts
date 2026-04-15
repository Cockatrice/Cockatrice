import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Data } from '@app/types';

export function concede(gameId: number): void {
  webClient.protobuf.sendGameCommand(gameId, Data.Command_Concede_ext, create(Data.Command_ConcedeSchema));
}
