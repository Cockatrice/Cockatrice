import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_FlipCardSchema, Command_FlipCard_ext } from 'generated/proto/command_flip_card_pb';
import { FlipCardParams } from 'types';

export function flipCard(gameId: number, params: FlipCardParams): void {
  BackendService.sendGameCommand(gameId, Command_FlipCard_ext, create(Command_FlipCardSchema, params));
}
