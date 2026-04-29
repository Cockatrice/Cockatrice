import { create } from '@bufbuild/protobuf';
import { Command_CreateToken_ext, Command_CreateTokenSchema, type CreateTokenParams } from '@app/generated';
import { WebClient } from '../../WebClient';

export function createToken(gameId: number, params: CreateTokenParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Command_CreateToken_ext, create(Command_CreateTokenSchema, params));
}
