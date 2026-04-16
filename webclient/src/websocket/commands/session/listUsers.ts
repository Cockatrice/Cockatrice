import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function listUsers(): void {
  WebClient.instance.protobuf.sendSessionCommand(Data.Command_ListUsers_ext, create(Data.Command_ListUsersSchema), {
    responseExt: Data.Response_ListUsers_ext,
    onSuccess: (response) => {
      WebClient.instance.response.session.updateUsers(response.userList);
    },
  });
}
