import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_CreateTokenSchema, Command_CreateToken_ext } from 'generated/proto/command_create_token_pb';
import { CreateTokenParams } from 'types';

export function createToken(gameId: number, params: CreateTokenParams): void {
  webClient.protobuf.sendGameCommand(gameId, Command_CreateToken_ext, create(Command_CreateTokenSchema, params));
}
