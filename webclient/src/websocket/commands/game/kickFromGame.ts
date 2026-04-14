import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_KickFromGameSchema, Command_KickFromGame_ext } from 'generated/proto/command_kick_from_game_pb';
import { KickFromGameParams } from 'types';

export function kickFromGame(gameId: number, params: KickFromGameParams): void {
  webClient.protobuf.sendGameCommand(gameId, Command_KickFromGame_ext, create(Command_KickFromGameSchema, params));
}
