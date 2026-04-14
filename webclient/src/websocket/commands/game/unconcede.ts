import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_UnconcedeSchema, Command_Unconcede_ext } from 'generated/proto/command_concede_pb';

export function unconcede(gameId: number): void {
  webClient.protobuf.sendGameCommand(gameId, Command_Unconcede_ext, create(Command_UnconcedeSchema));
}
