import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { Data } from '@app/types';

export function createArrow(gameId: number, params: Data.CreateArrowParams): void {
  webClient.protobuf.sendGameCommand(gameId, Data.Command_CreateArrow_ext, create(Data.Command_CreateArrowSchema, params));
}
