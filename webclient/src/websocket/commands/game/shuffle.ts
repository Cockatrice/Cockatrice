import { create } from '@bufbuild/protobuf';
import { Command_Shuffle_ext, Command_ShuffleSchema, type ShuffleParams } from '@app/generated';
import { WebClient } from '../../WebClient';

export function shuffle(gameId: number, params: ShuffleParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Command_Shuffle_ext, create(Command_ShuffleSchema, params));
}
