import { create } from '@bufbuild/protobuf';
import { Command_DeleteArrow_ext, Command_DeleteArrowSchema, type DeleteArrowParams } from '@app/generated';
import { WebClient } from '../../WebClient';

export function deleteArrow(gameId: number, params: DeleteArrowParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Command_DeleteArrow_ext, create(Command_DeleteArrowSchema, params));
}
