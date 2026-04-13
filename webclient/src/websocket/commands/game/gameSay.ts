import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_GameSaySchema, Command_GameSay_ext } from 'generated/proto/command_game_say_pb';
import { GameSayParams } from 'types';

export function gameSay(gameId: number, params: GameSayParams): void {
  BackendService.sendGameCommand(gameId, Command_GameSay_ext, create(Command_GameSaySchema, params));
}
