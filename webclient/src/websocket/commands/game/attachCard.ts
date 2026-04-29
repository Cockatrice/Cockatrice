import { create } from '@bufbuild/protobuf';
import { Command_AttachCard_ext, Command_AttachCardSchema, type AttachCardParams } from '@app/generated';
import { WebClient } from '../../WebClient';

export function attachCard(gameId: number, params: AttachCardParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Command_AttachCard_ext, create(Command_AttachCardSchema, params));
}
