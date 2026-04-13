import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_ListUsers_ext, Command_ListUsersSchema } from 'generated/proto/session_commands_pb';
import { SessionPersistence } from '../../persistence';
import { Response_ListUsers_ext } from 'generated/proto/response_list_users_pb';

export function listUsers(): void {
  BackendService.sendSessionCommand(Command_ListUsers_ext, create(Command_ListUsersSchema), {
    responseExt: Response_ListUsers_ext,
    onSuccess: (response) => {
      SessionPersistence.updateUsers(response.userList);
    },
  });
}
