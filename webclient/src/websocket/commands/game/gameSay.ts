import { create } from '@bufbuild/protobuf';
import { Command_GameSay_ext, Command_GameSaySchema, type GameSayParams } from '@app/generated';
import { WebClient } from '../../WebClient';

export function gameSay(gameId: number, params: GameSayParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Command_GameSay_ext, create(Command_GameSaySchema, params));
}
