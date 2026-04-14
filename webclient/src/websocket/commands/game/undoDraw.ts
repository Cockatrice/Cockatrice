import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_UndoDrawSchema, Command_UndoDraw_ext } from 'generated/proto/command_undo_draw_pb';

export function undoDraw(gameId: number): void {
  webClient.protobuf.sendGameCommand(gameId, Command_UndoDraw_ext, create(Command_UndoDrawSchema));
}
