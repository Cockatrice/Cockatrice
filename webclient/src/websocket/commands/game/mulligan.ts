import { create } from '@bufbuild/protobuf';
import { Command_Mulligan_ext, Command_MulliganSchema, type MulliganParams } from '@app/generated';
import { WebClient } from '../../WebClient';

export function mulligan(gameId: number, params: MulliganParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Command_Mulligan_ext, create(Command_MulliganSchema, params));
}
