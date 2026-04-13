import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_SetCardAttrSchema, Command_SetCardAttr_ext } from 'generated/proto/command_set_card_attr_pb';
import { SetCardAttrParams } from 'types';

export function setCardAttr(gameId: number, params: SetCardAttrParams): void {
  BackendService.sendGameCommand(gameId, Command_SetCardAttr_ext, create(Command_SetCardAttrSchema, params));
}
