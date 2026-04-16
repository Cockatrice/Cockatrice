import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function createArrow(gameId: number, params: Data.CreateArrowParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Data.Command_CreateArrow_ext, create(Data.Command_CreateArrowSchema, params));
}
