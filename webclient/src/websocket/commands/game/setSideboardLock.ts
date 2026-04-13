import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_SetSideboardLockSchema, Command_SetSideboardLock_ext } from 'generated/proto/command_set_sideboard_lock_pb';
import { SetSideboardLockParams } from 'types';

export function setSideboardLock(gameId: number, params: SetSideboardLockParams): void {
  BackendService.sendGameCommand(gameId, Command_SetSideboardLock_ext, create(Command_SetSideboardLockSchema, params));
}
