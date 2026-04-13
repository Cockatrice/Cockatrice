import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_SetCardCounterSchema, Command_SetCardCounter_ext } from 'generated/proto/command_set_card_counter_pb';
import { SetCardCounterParams } from 'types';

export function setCardCounter(gameId: number, params: SetCardCounterParams): void {
  BackendService.sendGameCommand(gameId, Command_SetCardCounter_ext, create(Command_SetCardCounterSchema, params));
}
