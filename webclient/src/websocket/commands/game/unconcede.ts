import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Data } from '@app/types';

export function unconcede(gameId: number): void {
  webClient.protobuf.sendGameCommand(gameId, Data.Command_Unconcede_ext, create(Data.Command_UnconcedeSchema));
}
