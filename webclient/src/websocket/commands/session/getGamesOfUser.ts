import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { SessionPersistence } from '../../persistence';
import { Data } from '@app/types';

export function getGamesOfUser(userName: string): void {
  webClient.protobuf.sendSessionCommand(Data.Command_GetGamesOfUser_ext, create(Data.Command_GetGamesOfUserSchema, { userName }), {
    responseExt: Data.Response_GetGamesOfUser_ext,
    onSuccess: (response) => {
      SessionPersistence.getGamesOfUser(userName, response);
    },
  });
}
