import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_ReverseTurnSchema, Command_ReverseTurn_ext } from 'generated/proto/command_reverse_turn_pb';

export function reverseTurn(gameId: number): void {
  webClient.protobuf.sendGameCommand(gameId, Command_ReverseTurn_ext, create(Command_ReverseTurnSchema));
}
