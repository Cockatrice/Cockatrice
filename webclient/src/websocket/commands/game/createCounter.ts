import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_CreateCounterSchema, Command_CreateCounter_ext } from 'generated/proto/command_create_counter_pb';
import { CreateCounterParams } from 'types';

export function createCounter(gameId: number, params: CreateCounterParams): void {
  BackendService.sendGameCommand(gameId, Command_CreateCounter_ext, create(Command_CreateCounterSchema, params));
}
