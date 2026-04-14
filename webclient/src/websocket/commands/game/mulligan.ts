import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_MulliganSchema, Command_Mulligan_ext } from 'generated/proto/command_mulligan_pb';
import { MulliganParams } from 'types';

export function mulligan(gameId: number, params: MulliganParams): void {
  webClient.protobuf.sendGameCommand(gameId, Command_Mulligan_ext, create(Command_MulliganSchema, params));
}
