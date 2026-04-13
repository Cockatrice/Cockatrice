import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_CreateArrowSchema, Command_CreateArrow_ext } from 'generated/proto/command_create_arrow_pb';
import { CreateArrowParams } from 'types';

export function createArrow(gameId: number, params: CreateArrowParams): void {
  BackendService.sendGameCommand(gameId, Command_CreateArrow_ext, create(Command_CreateArrowSchema, params));
}
