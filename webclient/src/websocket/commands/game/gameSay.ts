import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_GameSaySchema, Command_GameSay_ext } from 'generated/proto/command_game_say_pb';
import { GameSayParams } from 'types';

export function gameSay(gameId: number, params: GameSayParams): void {
  webClient.protobuf.sendGameCommand(gameId, Command_GameSay_ext, create(Command_GameSaySchema, params));
}
