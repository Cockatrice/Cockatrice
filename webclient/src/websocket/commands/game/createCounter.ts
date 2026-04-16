import { create } from '@bufbuild/protobuf';
import { Command_CreateCounter_ext, Command_CreateCounterSchema, type CreateCounterParams } from '@app/generated';
import { WebClient } from '../../WebClient';

export function createCounter(gameId: number, params: CreateCounterParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Command_CreateCounter_ext, create(Command_CreateCounterSchema, params));
}
