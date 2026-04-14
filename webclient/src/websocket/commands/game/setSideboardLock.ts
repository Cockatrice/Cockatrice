import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_SetSideboardLockSchema, Command_SetSideboardLock_ext } from 'generated/proto/command_set_sideboard_lock_pb';
import { SetSideboardLockParams } from 'types';

export function setSideboardLock(gameId: number, params: SetSideboardLockParams): void {
  webClient.protobuf.sendGameCommand(gameId, Command_SetSideboardLock_ext, create(Command_SetSideboardLockSchema, params));
}
