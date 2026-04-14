import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_FlipCardSchema, Command_FlipCard_ext } from 'generated/proto/command_flip_card_pb';
import { FlipCardParams } from 'types';

export function flipCard(gameId: number, params: FlipCardParams): void {
  webClient.protobuf.sendGameCommand(gameId, Command_FlipCard_ext, create(Command_FlipCardSchema, params));
}
