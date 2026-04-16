import { create } from '@bufbuild/protobuf';
import { Command_SetCardAttr_ext, Command_SetCardAttrSchema, type SetCardAttrParams } from '@app/generated';
import { WebClient } from '../../WebClient';

export function setCardAttr(gameId: number, params: SetCardAttrParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Command_SetCardAttr_ext, create(Command_SetCardAttrSchema, params));
}
