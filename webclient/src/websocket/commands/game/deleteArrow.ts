import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_DeleteArrowSchema, Command_DeleteArrow_ext } from 'generated/proto/command_delete_arrow_pb';
import { DeleteArrowParams } from 'types';

export function deleteArrow(gameId: number, params: DeleteArrowParams): void {
  BackendService.sendGameCommand(gameId, Command_DeleteArrow_ext, create(Command_DeleteArrowSchema, params));
}
