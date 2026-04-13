import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_ReadyStartSchema, Command_ReadyStart_ext } from 'generated/proto/command_ready_start_pb';
import { ReadyStartParams } from 'types';

export function readyStart(gameId: number, params: ReadyStartParams): void {
  BackendService.sendGameCommand(gameId, Command_ReadyStart_ext, create(Command_ReadyStartSchema, params));
}
