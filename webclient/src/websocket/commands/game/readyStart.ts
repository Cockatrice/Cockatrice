import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_ReadyStartSchema, Command_ReadyStart_ext } from 'generated/proto/command_ready_start_pb';
import { ReadyStartParams } from 'types';

export function readyStart(gameId: number, params: ReadyStartParams): void {
  webClient.protobuf.sendGameCommand(gameId, Command_ReadyStart_ext, create(Command_ReadyStartSchema, params));
}
