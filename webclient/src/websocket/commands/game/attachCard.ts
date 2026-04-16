import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function attachCard(gameId: number, params: Data.AttachCardParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Data.Command_AttachCard_ext, create(Data.Command_AttachCardSchema, params));
}
