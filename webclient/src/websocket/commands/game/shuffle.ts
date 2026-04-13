import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_ShuffleSchema, Command_Shuffle_ext } from 'generated/proto/command_shuffle_pb';
import { ShuffleParams } from 'types';

export function shuffle(gameId: number, params: ShuffleParams): void {
  BackendService.sendGameCommand(gameId, Command_Shuffle_ext, create(Command_ShuffleSchema, params));
}
