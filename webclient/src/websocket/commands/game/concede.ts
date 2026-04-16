import { create } from '@bufbuild/protobuf';
import { Command_Concede_ext, Command_ConcedeSchema } from '@app/generated';
import { WebClient } from '../../WebClient';

export function concede(gameId: number): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Command_Concede_ext, create(Command_ConcedeSchema));
}
