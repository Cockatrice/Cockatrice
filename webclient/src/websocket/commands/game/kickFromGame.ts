import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { Data } from '@app/types';

export function kickFromGame(gameId: number, params: Data.KickFromGameParams): void {
  webClient.protobuf.sendGameCommand(gameId, Data.Command_KickFromGame_ext, create(Data.Command_KickFromGameSchema, params));
}
