import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Command_ListUsers_ext, Command_ListUsersSchema, Response_ListUsers_ext } from '@app/generated';

export function listUsers(): void {
  WebClient.instance.protobuf.sendSessionCommand(Command_ListUsers_ext, create(Command_ListUsersSchema), {
    responseExt: Response_ListUsers_ext,
    onSuccess: (response) => {
      WebClient.instance.response.session.updateUsers(response.userList);
    },
  });
}
