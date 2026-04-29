import { create } from '@bufbuild/protobuf';
import { Command_NextTurn_ext, Command_NextTurnSchema } from '@app/generated';
import { WebClient } from '../../WebClient';

export function nextTurn(gameId: number): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Command_NextTurn_ext, create(Command_NextTurnSchema));
}
