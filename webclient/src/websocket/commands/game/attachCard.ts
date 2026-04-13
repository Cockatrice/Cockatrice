import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_AttachCardSchema, Command_AttachCard_ext } from 'generated/proto/command_attach_card_pb';
import { AttachCardParams } from 'types';

export function attachCard(gameId: number, params: AttachCardParams): void {
  BackendService.sendGameCommand(gameId, Command_AttachCard_ext, create(Command_AttachCardSchema, params));
}
