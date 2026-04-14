import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_SetCardCounterSchema, Command_SetCardCounter_ext } from 'generated/proto/command_set_card_counter_pb';
import { SetCardCounterParams } from 'types';

export function setCardCounter(gameId: number, params: SetCardCounterParams): void {
  webClient.protobuf.sendGameCommand(gameId, Command_SetCardCounter_ext, create(Command_SetCardCounterSchema, params));
}
