import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { SessionPersistence } from '../../persistence';
import { Data } from '@app/types';

export function listUsers(): void {
  webClient.protobuf.sendSessionCommand(Data.Command_ListUsers_ext, create(Data.Command_ListUsersSchema), {
    responseExt: Data.Response_ListUsers_ext,
    onSuccess: (response) => {
      SessionPersistence.updateUsers(response.userList);
    },
  });
}
