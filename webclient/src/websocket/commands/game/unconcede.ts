import { create } from '@bufbuild/protobuf';
import { Command_Unconcede_ext, Command_UnconcedeSchema } from '@app/generated';
import { WebClient } from '../../WebClient';

export function unconcede(gameId: number): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Command_Unconcede_ext, create(Command_UnconcedeSchema));
}
