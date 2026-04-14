import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_SetCounterSchema, Command_SetCounter_ext } from 'generated/proto/command_set_counter_pb';
import { SetCounterParams } from 'types';

export function setCounter(gameId: number, params: SetCounterParams): void {
  webClient.protobuf.sendGameCommand(gameId, Command_SetCounter_ext, create(Command_SetCounterSchema, params));
}
