import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_IncCounterSchema, Command_IncCounter_ext } from 'generated/proto/command_inc_counter_pb';
import { IncCounterParams } from 'types';

export function incCounter(gameId: number, params: IncCounterParams): void {
  BackendService.sendGameCommand(gameId, Command_IncCounter_ext, create(Command_IncCounterSchema, params));
}
