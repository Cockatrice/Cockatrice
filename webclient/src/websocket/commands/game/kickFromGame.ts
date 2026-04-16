import { create } from '@bufbuild/protobuf';
import { Command_KickFromGame_ext, Command_KickFromGameSchema, type KickFromGameParams } from '@app/generated';
import { WebClient } from '../../WebClient';

export function kickFromGame(gameId: number, params: KickFromGameParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Command_KickFromGame_ext, create(Command_KickFromGameSchema, params));
}
