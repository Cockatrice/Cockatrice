import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_CreateTokenSchema, Command_CreateToken_ext } from 'generated/proto/command_create_token_pb';
import { CreateTokenParams } from 'types';

export function createToken(gameId: number, params: CreateTokenParams): void {
  BackendService.sendGameCommand(gameId, Command_CreateToken_ext, create(Command_CreateTokenSchema, params));
}
