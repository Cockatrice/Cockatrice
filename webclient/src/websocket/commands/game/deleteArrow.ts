import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function deleteArrow(gameId: number, params: Data.DeleteArrowParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Data.Command_DeleteArrow_ext, create(Data.Command_DeleteArrowSchema, params));
}
