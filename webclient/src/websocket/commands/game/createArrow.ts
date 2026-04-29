import { create } from '@bufbuild/protobuf';
import { Command_CreateArrow_ext, Command_CreateArrowSchema, type CreateArrowParams } from '@app/generated';
import { WebClient } from '../../WebClient';

export function createArrow(gameId: number, params: CreateArrowParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Command_CreateArrow_ext, create(Command_CreateArrowSchema, params));
}
