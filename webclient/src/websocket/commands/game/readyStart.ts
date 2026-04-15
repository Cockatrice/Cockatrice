import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { Data } from '@app/types';

export function readyStart(gameId: number, params: Data.ReadyStartParams): void {
  webClient.protobuf.sendGameCommand(gameId, Data.Command_ReadyStart_ext, create(Data.Command_ReadyStartSchema, params));
}
