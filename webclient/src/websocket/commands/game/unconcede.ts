import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';
import { Data } from '@app/types';

export function unconcede(gameId: number): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Data.Command_Unconcede_ext, create(Data.Command_UnconcedeSchema));
}
