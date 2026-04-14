import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_NextTurnSchema, Command_NextTurn_ext } from 'generated/proto/command_next_turn_pb';

export function nextTurn(gameId: number): void {
  webClient.protobuf.sendGameCommand(gameId, Command_NextTurn_ext, create(Command_NextTurnSchema));
}
