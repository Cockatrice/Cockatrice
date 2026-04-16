import { create } from '@bufbuild/protobuf';
import { Command_SetCardCounter_ext, Command_SetCardCounterSchema, type SetCardCounterParams } from '@app/generated';
import { WebClient } from '../../WebClient';

export function setCardCounter(gameId: number, params: SetCardCounterParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Command_SetCardCounter_ext, create(Command_SetCardCounterSchema, params));
}
