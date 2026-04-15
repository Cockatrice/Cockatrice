import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { Data } from '@app/types';

export function setCardAttr(gameId: number, params: Data.SetCardAttrParams): void {
  webClient.protobuf.sendGameCommand(gameId, Data.Command_SetCardAttr_ext, create(Data.Command_SetCardAttrSchema, params));
}
