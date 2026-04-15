import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { Data } from '@app/types';

export function createToken(gameId: number, params: Data.CreateTokenParams): void {
  webClient.protobuf.sendGameCommand(gameId, Data.Command_CreateToken_ext, create(Data.Command_CreateTokenSchema, params));
}
