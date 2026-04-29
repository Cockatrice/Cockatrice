import { create } from '@bufbuild/protobuf';
import { Command_SetCounter_ext, Command_SetCounterSchema, type SetCounterParams } from '@app/generated';
import { WebClient } from '../../WebClient';

export function setCounter(gameId: number, params: SetCounterParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Command_SetCounter_ext, create(Command_SetCounterSchema, params));
}
