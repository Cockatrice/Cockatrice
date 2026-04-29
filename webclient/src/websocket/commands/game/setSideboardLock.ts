import { create } from '@bufbuild/protobuf';
import { Command_SetSideboardLock_ext, Command_SetSideboardLockSchema, type SetSideboardLockParams } from '@app/generated';
import { WebClient } from '../../WebClient';

export function setSideboardLock(gameId: number, params: SetSideboardLockParams): void {
  WebClient.instance.protobuf.sendGameCommand(
    gameId,
    Command_SetSideboardLock_ext,
    create(Command_SetSideboardLockSchema, params)
  );
}
