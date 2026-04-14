import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_IncCounterSchema, Command_IncCounter_ext } from 'generated/proto/command_inc_counter_pb';
import { IncCounterParams } from 'types';

export function incCounter(gameId: number, params: IncCounterParams): void {
  webClient.protobuf.sendGameCommand(gameId, Command_IncCounter_ext, create(Command_IncCounterSchema, params));
}
