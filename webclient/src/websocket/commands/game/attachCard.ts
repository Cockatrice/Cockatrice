import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_AttachCardSchema, Command_AttachCard_ext } from 'generated/proto/command_attach_card_pb';
import { AttachCardParams } from 'types';

export function attachCard(gameId: number, params: AttachCardParams): void {
  webClient.protobuf.sendGameCommand(gameId, Command_AttachCard_ext, create(Command_AttachCardSchema, params));
}
