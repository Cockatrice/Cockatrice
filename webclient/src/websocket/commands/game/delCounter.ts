import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_DelCounterSchema, Command_DelCounter_ext } from 'generated/proto/command_del_counter_pb';
import { DelCounterParams } from 'types';

export function delCounter(gameId: number, params: DelCounterParams): void {
  BackendService.sendGameCommand(gameId, Command_DelCounter_ext, create(Command_DelCounterSchema, params));
}
