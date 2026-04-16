import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function flipCard(gameId: number, params: Data.FlipCardParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Data.Command_FlipCard_ext, create(Data.Command_FlipCardSchema, params));
}
