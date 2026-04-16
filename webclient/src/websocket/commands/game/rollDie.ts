import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function rollDie(gameId: number, params: Data.RollDieParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Data.Command_RollDie_ext, create(Data.Command_RollDieSchema, params));
}
