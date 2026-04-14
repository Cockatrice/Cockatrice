import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_SetCardAttrSchema, Command_SetCardAttr_ext } from 'generated/proto/command_set_card_attr_pb';
import { SetCardAttrParams } from 'types';

export function setCardAttr(gameId: number, params: SetCardAttrParams): void {
  webClient.protobuf.sendGameCommand(gameId, Command_SetCardAttr_ext, create(Command_SetCardAttrSchema, params));
}
