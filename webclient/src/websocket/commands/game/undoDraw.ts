import { create } from '@bufbuild/protobuf';
import { Command_UndoDraw_ext, Command_UndoDrawSchema } from '@app/generated';
import { WebClient } from '../../WebClient';

export function undoDraw(gameId: number): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Command_UndoDraw_ext, create(Command_UndoDrawSchema));
}
