import { create } from '@bufbuild/protobuf';
import { Command_DelCounter_ext, Command_DelCounterSchema, type DelCounterParams } from '@app/generated';
import { WebClient } from '../../WebClient';

export function delCounter(gameId: number, params: DelCounterParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Command_DelCounter_ext, create(Command_DelCounterSchema, params));
}
