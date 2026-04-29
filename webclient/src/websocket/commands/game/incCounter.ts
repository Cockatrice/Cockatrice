import { create } from '@bufbuild/protobuf';
import { Command_IncCounter_ext, Command_IncCounterSchema, type IncCounterParams } from '@app/generated';
import { WebClient } from '../../WebClient';

export function incCounter(gameId: number, params: IncCounterParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Command_IncCounter_ext, create(Command_IncCounterSchema, params));
}
