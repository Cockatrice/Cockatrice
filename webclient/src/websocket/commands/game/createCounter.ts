import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_CreateCounterSchema, Command_CreateCounter_ext } from 'generated/proto/command_create_counter_pb';
import { CreateCounterParams } from 'types';

export function createCounter(gameId: number, params: CreateCounterParams): void {
  webClient.protobuf.sendGameCommand(gameId, Command_CreateCounter_ext, create(Command_CreateCounterSchema, params));
}
