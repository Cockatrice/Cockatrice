import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_DeleteArrowSchema, Command_DeleteArrow_ext } from 'generated/proto/command_delete_arrow_pb';
import { DeleteArrowParams } from 'types';

export function deleteArrow(gameId: number, params: DeleteArrowParams): void {
  webClient.protobuf.sendGameCommand(gameId, Command_DeleteArrow_ext, create(Command_DeleteArrowSchema, params));
}
