import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { Data } from '@app/types';

export function attachCard(gameId: number, params: Data.AttachCardParams): void {
  webClient.protobuf.sendGameCommand(gameId, Data.Command_AttachCard_ext, create(Data.Command_AttachCardSchema, params));
}
