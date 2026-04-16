import { create } from '@bufbuild/protobuf';
import { Command_RollDie_ext, Command_RollDieSchema, type RollDieParams } from '@app/generated';
import { WebClient } from '../../WebClient';

export function rollDie(gameId: number, params: RollDieParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Command_RollDie_ext, create(Command_RollDieSchema, params));
}
