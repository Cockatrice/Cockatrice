import { create } from '@bufbuild/protobuf';
import { Command_ReadyStart_ext, Command_ReadyStartSchema, type ReadyStartParams } from '@app/generated';
import { WebClient } from '../../WebClient';

export function readyStart(gameId: number, params: ReadyStartParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Command_ReadyStart_ext, create(Command_ReadyStartSchema, params));
}
