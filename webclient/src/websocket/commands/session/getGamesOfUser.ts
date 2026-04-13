import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_GetGamesOfUser_ext, Command_GetGamesOfUserSchema } from 'generated/proto/session_commands_pb';
import { SessionPersistence } from '../../persistence';
import { Response_GetGamesOfUser_ext } from 'generated/proto/response_get_games_of_user_pb';

export function getGamesOfUser(userName: string): void {
  BackendService.sendSessionCommand(Command_GetGamesOfUser_ext, create(Command_GetGamesOfUserSchema, { userName }), {
    responseExt: Response_GetGamesOfUser_ext,
    onSuccess: (response) => {
      SessionPersistence.getGamesOfUser(userName, response);
    },
  });
}
