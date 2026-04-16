import { create } from '@bufbuild/protobuf';
import { Command_IncCardCounter_ext, Command_IncCardCounterSchema, type IncCardCounterParams } from '@app/generated';
import { WebClient } from '../../WebClient';

export function incCardCounter(gameId: number, params: IncCardCounterParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Command_IncCardCounter_ext, create(Command_IncCardCounterSchema, params));
}
