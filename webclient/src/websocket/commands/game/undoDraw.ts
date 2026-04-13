import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_UndoDrawSchema, Command_UndoDraw_ext } from 'generated/proto/command_undo_draw_pb';

export function undoDraw(gameId: number): void {
  BackendService.sendGameCommand(gameId, Command_UndoDraw_ext, create(Command_UndoDrawSchema));
}
