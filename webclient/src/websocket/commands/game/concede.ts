import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_ConcedeSchema, Command_Concede_ext } from 'generated/proto/command_concede_pb';

export function concede(gameId: number): void {
  webClient.protobuf.sendGameCommand(gameId, Command_Concede_ext, create(Command_ConcedeSchema));
}
