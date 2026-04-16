import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Command_GetGamesOfUser_ext, Command_GetGamesOfUserSchema, Response_GetGamesOfUser_ext } from '@app/generated';

export function getGamesOfUser(userName: string): void {
  WebClient.instance.protobuf.sendSessionCommand(Command_GetGamesOfUser_ext, create(Command_GetGamesOfUserSchema, { userName }), {
    responseExt: Response_GetGamesOfUser_ext,
    onSuccess: (response) => {
      WebClient.instance.response.session.getGamesOfUser(userName, response);
    },
  });
}
