import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';
import { Data } from '@app/types';

export function undoDraw(gameId: number): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Data.Command_UndoDraw_ext, create(Data.Command_UndoDrawSchema));
}
