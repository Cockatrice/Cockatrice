import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_ShuffleSchema, Command_Shuffle_ext } from 'generated/proto/command_shuffle_pb';
import { ShuffleParams } from 'types';

export function shuffle(gameId: number, params: ShuffleParams): void {
  webClient.protobuf.sendGameCommand(gameId, Command_Shuffle_ext, create(Command_ShuffleSchema, params));
}
