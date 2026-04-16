import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function getGamesOfUser(userName: string): void {
  WebClient.instance.protobuf.sendSessionCommand(Data.Command_GetGamesOfUser_ext, create(Data.Command_GetGamesOfUserSchema, { userName }), {
    responseExt: Data.Response_GetGamesOfUser_ext,
    onSuccess: (response) => {
      WebClient.instance.response.session.getGamesOfUser(userName, response);
    },
  });
}
