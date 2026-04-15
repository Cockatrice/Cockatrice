import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { Data } from '@app/types';

export function gameSay(gameId: number, params: Data.GameSayParams): void {
  webClient.protobuf.sendGameCommand(gameId, Data.Command_GameSay_ext, create(Data.Command_GameSaySchema, params));
}
