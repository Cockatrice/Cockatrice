import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_IncCardCounterSchema, Command_IncCardCounter_ext } from 'generated/proto/command_inc_card_counter_pb';
import { IncCardCounterParams } from 'types';

export function incCardCounter(gameId: number, params: IncCardCounterParams): void {
  webClient.protobuf.sendGameCommand(gameId, Command_IncCardCounter_ext, create(Command_IncCardCounterSchema, params));
}
