import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function createToken(gameId: number, params: Data.CreateTokenParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Data.Command_CreateToken_ext, create(Data.Command_CreateTokenSchema, params));
}
